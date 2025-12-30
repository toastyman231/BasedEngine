#!/usr/bin/env python3

import os
import sys
import globals
import traceback

CI_MODE = False

import argparse
import shutil
import subprocess
import time
from pathlib import Path

import multiprocessing
from concurrent.futures import ThreadPoolExecutor, as_completed
import threading
from datetime import datetime
import signal

def ensure_dependencies():
    import importlib
    import subprocess
    import sys

    required = {
        "PIL": "Pillow",
        "colorama": "colorama",
    }

    missing = []

    for module, package in required.items():
        try:
            importlib.import_module(module)
        except ImportError:
            missing.append(package)

    if not missing:
        return

    print("The following required Python packages are missing:")
    for pkg in missing:
        print(f"  - {pkg}")

    try:
        choice = input("\nWould you like to install them now? [Y/n]: ").strip().lower()
    except EOFError:
        choice = "y"

    if choice not in ("", "y", "yes"):
        print("Dependencies not installed. Exiting.")
        sys.exit(1)

    print("\nInstalling dependencies...\n")

    try:
        subprocess.check_call(
            [sys.executable, "-m", "pip", "install", *missing]
        )
    except subprocess.CalledProcessError:
        print("\nFailed to install dependencies. Please install them manually:")
        print(f"  pip install {' '.join(missing)}")
        sys.exit(1)

    print("\nDependencies installed successfully. Restarting...\n")
    os.execv(sys.executable, [sys.executable] + sys.argv)

ensure_dependencies();

from PIL import Image
from colorama import init as colorama_init
from colorama import Fore
from colorama import Style

COMPRESSONATOR_CLI = Path(globals.ENGINE_DIR).resolve() / "tools" / "bin" / globals.GetOSDir() / "Compressonator" / "compressonatorcli{}".format(globals.GetOSExtension())
KTX_CLI = Path(globals.ENGINE_DIR).resolve() / "tools" / "bin" / globals.GetOSDir() / "KTX" / "ktx{}".format(globals.GetOSExtension())

BULLET = "•"
CHECK = "✓"
WARN = "⚠"
BAR_FULL = "█"
BAR_EMPTY = "░"

def supports_utf8():
    try:
        return sys.stdout.encoding and "utf" in sys.stdout.encoding.lower()
    except Exception:
        return False

UTF8 = supports_utf8()

def get_project_name(path, override_name):
    if override_name:
        return override_name
    return path.name

def make_output_path(input_path, project_root, destination_root, extension_override=""):
    relative = input_path.relative_to(project_root)
    
    output_path = destination_root / relative
    
    # Remove PostBuildCopy directories from path
    parts = list(output_path.parts)
    filtered_parts = [p for p in parts if p not in ["PostBuildCopy", "PostBuildCopy_windows"]]
    output_path = Path(*filtered_parts) if filtered_parts else output_path
    
    if extension_override:
        output_path = output_path.with_suffix(extension_override)
    
    return output_path

def is_normal_map(path, normal_identifiers):
    filename = path.name.lower()
    
    identifiers = normal_identifiers if normal_identifiers else ["normal"]
    
    for identifier in identifiers:
        if identifier.lower() in filename:
            return True
    
    return False

def get_compressonator_format(path, is_normal, channels, target):
    """Get the compression format for Compressonator"""
    if target == "ios":
        return "PVRTC1_4"
    elif target == "android":
        return "ETC2_RGBA"
    elif target == "web":
        return "BC3"
    else:  # desktop or legacy-desktop
        if is_normal:
            return "BC3" if target == "legacy-desktop" else "BC5"
        elif channels == 1:
            return "BC4"
        else:
            return "BC3" if target == "legacy-desktop" else "BC7"

def prepare_normal_map(image):
    """Prepare normal map by copying G channel to A channel"""
    img_array = image.convert("RGBA")
    pixels = img_array.load()
    width, height = img_array.size
    
    for y in range(height):
        for x in range(width):
            r, g, b, a = pixels[x, y]
            # Copy G channel to A channel for BC5 compression
            pixels[x, y] = (r, g, b, g)
    
    return img_array

def handle_texture_compression(path, source_path, destination_path, args, progress_callback=None, error_logger=None):
    if not path.is_file() or ".ico" in path.name.lower():
        return False
    
    wrote_successfully = False
    
    output_path = make_output_path(path, source_path, destination_path, ".ktx2")
    original_path = path;
    
    # Skip if up to date
    if not args.force_textures and output_path.exists():
        if path.stat().st_mtime < output_path.stat().st_mtime:
            if progress_callback:
                progress_callback(path.name, "skipped")
            return True
    
    texture_is_normal_map = is_normal_map(path, args.normal_identifiers)
    
    start_time = time.time()
    
    try:
        # Load image with PIL to get info
        # Suppress PIL warnings for non-image files
        import warnings
        with warnings.catch_warnings():
            warnings.simplefilter("ignore")
            try:
                img = Image.open(path)
                img.load()  # Force load to trigger any errors
                if path.suffix.lower() in [".jpg", ".jpeg"]:
                    # Some JPEGs fail in CLI; convert to PNG without changing channels
                    temp_path = path.with_suffix(".tmp.png")
                    img.save(temp_path)
                    path = temp_path
            except Exception:
                # Not an image file, skip silently
                return False
        
        if progress_callback:
            progress_callback(path.name, "compressing")
        
        # Determine channels
        if img.mode == 'L':
            channels = 1
        elif img.mode == 'RGB':
            channels = 3
        else:
            channels = 4
        
        # Get compression format
        compression_format = get_compressonator_format(path, texture_is_normal_map, channels, args.target)
        
        # Create output directory
        output_path.parent.mkdir(parents=True, exist_ok=True)
        
        # Build Compressonator command
        cmd = [str(COMPRESSONATOR_CLI)]
        
        # Input file
        cmd.extend([str(path.resolve())])
        
        # Output file
        cmd.extend([str(output_path.resolve())])
        
        # Compression format
        if not args.no_transcode:
            cmd.extend(["-fd", compression_format])
        
        # Quality setting
        cmd.extend(["-Quality", str(args.quality)])
        
        # Mipmap generation
        if not args.no_mips:
            # Generate all mipmaps now during compression
            cmd.extend(["-mipsize", "4"])     # Generate down to 1x1
        
        # Use GPU encoding for massive speedup
        cmd.extend(["-EncodeWith", "GPU"])
        
        # Output format as KTX2
        cmd.extend(["-UseGPUDecompress"])  # Use GPU decompression path
        
        # Let Compressonator use optimal thread count
        num_threads = multiprocessing.cpu_count()
        cmd.extend(["-NumThreads", str(num_threads)])
        
        # Run Compressonator
        result = subprocess.run(cmd, capture_output=True, text=True, encoding="utf-8")

        if original_path != path and path.exists():
            path.unlink()
        
        if result.returncode != 0:
            if error_logger:
                error_logger(f"Failed to compress {path.name}:\n"
                           f"  Command: {' '.join(cmd)}\n"
                           f"  Return code: {result.returncode}\n"
                           f"  Stdout: {result.stdout}\n"
                           f"  Stderr: {result.stderr}\n")
            if progress_callback:
                progress_callback(path.name, "failed")
            return False
        
        # Check if output file was actually created
        if not output_path.exists():
            if error_logger:
                error_logger(f"Output file {output_path.name} was not created for {path.name}\n")
            if progress_callback:
                progress_callback(path.name, "failed")
            return False
        
        wrote_successfully = True
        
        # Apply zstandard compression if requested using ktx tool
        if not args.no_deflate and output_path.exists():
            deflate_cmd = [str(KTX_CLI), "deflate", "--zstd", str(args.deflate_level), 
                          str(output_path), str(output_path)]
            deflate_result = subprocess.run(deflate_cmd, capture_output=True, text=True, encoding="utf-8")
            if deflate_result.returncode != 0:
                if error_logger:
                    error_logger(f"Failed to deflate {output_path.name}:\n"
                               f"  Command: {' '.join(deflate_cmd)}\n"
                               f"  Return code: {deflate_result.returncode}\n"
                               f"  Stderr: {deflate_result.stderr}\n")
                if progress_callback:
                    progress_callback(path.name, "deflate_failed")
        
        elapsed = time.time() - start_time
        if progress_callback:
            progress_callback(path.name, "completed", elapsed)
        
        return True
        
    except Exception as e:
        if error_logger:
            error_logger(f"Exception while processing {path.name}: {str(e)}\n       Path checked: {str(path)}\n")
            error_logger(f"{traceback.format_exc()}")
        # Silent failure for non-image files
        return wrote_successfully

def handle_copy(source, dest, progress_callback=None, error_logger=None):
    if not source.is_file():
        return False
    
    if progress_callback:
        progress_callback(source.name, "copying")
    
    dest.parent.mkdir(parents=True, exist_ok=True)
    
    try:
        if sys.platform == "win32":
            # Use robocopy on Windows
            cmd = ["robocopy", str(source.parent), str(dest.parent), source.name, "/mt"]
            subprocess.run(cmd, capture_output=True)
        else:
            # Use cp on Unix-like systems
            shutil.copy2(source, dest)
        
        if progress_callback:
            progress_callback(source.name, "copied")
        return True
    except Exception as e:
        if error_logger:
            error_logger(f"Failed to copy {source.name}: {str(e)}\n")
        if progress_callback:
            progress_callback(source.name, "copy_failed")
        return False

def handle_file(path, source_path, destination_path, args, progress_callback=None, error_logger=None):
    # Try texture compression first
    handled = handle_texture_compression(path, source_path, destination_path, args, progress_callback, error_logger)
    
    # If not a texture, just copy it
    if not handled:
        dest = make_output_path(path, source_path, destination_path)
        handled = handle_copy(path, dest, progress_callback, error_logger)
    
    return handled

def main():
    global BULLET, CHECK, WARN, BAR_FULL, BAR_EMPTY, CI_MODE
    
    parser = argparse.ArgumentParser(
        description="Build tool for processing textures and copying assets",
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    
    parser.add_argument("-i", "--input", required=True, help="Root directory of the project to build")
    parser.add_argument("-c", "--config", default="Debug", choices=["Debug", "Release"], 
                       help="Debug or Release configuration")
    parser.add_argument("-q", "--quality", type=float, default=0, 
                       help="Compression quality. Value between 0 and 1 where 0 = Fastest, 1 = Slowest")
    parser.add_argument("-l", "--deflate-level", type=int, default=10,
                       help="Zstandard supercompression level. Range: 1-22")
    parser.add_argument("-t", "--target", default="desktop",
                       choices=["desktop", "legacy-desktop", "web", "ios", "android"],
                       help="Target platform for texture compression")
    parser.add_argument("-j", "--jobs", type=int, default=8,
                       help="Number of parallel compression jobs (default: 8, GPU encoding allows more parallelism)")
    # Single character flags above, full word flags below
    parser.add_argument("--no-mips", action="store_true", 
                       help="Disable automatic mip generation")
    parser.add_argument("--no-transcode", action="store_true",
                       help="Prevent transcoding into GPU-specific formats")
    parser.add_argument("--no-deflate", action="store_true",
                       help="Disable Zstandard supercompression")
    parser.add_argument("--force-textures", action="store_true",
                       help="Force build all textures, even if they're up to date")
    parser.add_argument("--override-name", default="",
                       help="Name to use when outputting baked assets")
    parser.add_argument("--normal-identifiers", nargs="+", default=None,
                       help="Strings to identify normal maps (default: normal)")
    parser.add_argument("--ci", action="store_true",
                        help="Run in CI / build-system mode (no colors, no unicode, no cursor control)")
    
    args = parser.parse_args()

    CI_MODE = args.ci;

    if CI_MODE:
        BULLET = "*"
        CHECK = "+"
        WARN = "!"
        BAR_FULL = "#"
        BAR_EMPTY = "-"
    
    # Validate arguments
    if args.quality < 0 or args.quality > 1:
        print("Quality level must be between 0 and 1", file=sys.stderr)
        sys.exit(1)
    
    if args.deflate_level < 1 or args.deflate_level > 22:
        print("Deflate level must be between 1 and 22", file=sys.stderr)
        sys.exit(1)
    
    source_path = Path(args.input)
    if not source_path.is_dir():
        print("Project directory must be a directory!", file=sys.stderr)
        sys.exit(1)
    
    project_name = get_project_name(source_path, args.override_name)
    destination_path = source_path / "bin" / args.config / project_name
    
    if not destination_path.exists():
        print(f"Output directory {destination_path} does not exist! Creating it now.")
        destination_path.mkdir(parents=True, exist_ok=True)
    
    # Set up error logging
    log_dir = source_path / "Logs"
    log_dir.mkdir(exist_ok=True)
    log_file = log_dir / f"build_errors_{datetime.now().strftime('%Y%m%d_%H%M%S')}.log"
    log_lock = threading.Lock()
    error_count = 0
    interrupted = False
    
    def log_error(message):
        nonlocal error_count
        error_count += 1
        with log_lock:
            with open(log_file, 'a', encoding='utf-8') as f:
                f.write(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] {message}\n")
    
    def handle_interrupt(signum, frame):
        nonlocal interrupted
        interrupted = True
        with log_lock:
            with open(log_file, 'a', encoding='utf-8') as f:
                f.write(f"\n[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] Build interrupted by user (Ctrl+C)\n")
        print(f"\n\n{Fore.RED}{WARN} Build interrupted! Log file saved to: {Style.RESET_ALL}{log_file}")
        sys.exit(1)
    
    # Register interrupt handler
    signal.signal(signal.SIGINT, handle_interrupt)
    
    # Build queue of paths to search
    paths_to_search = []
    if sys.platform == "win32":
        paths_to_search.append(source_path / "PostBuildCopy_windows")
    else:
        paths_to_search.append(source_path / "PostBuildCopy")
    paths_to_search.append(source_path / "Assets")
    
    start_time = time.time()
    files_processed = 0
    
    # Collect all files to process
    all_files = []
    while paths_to_search:
        current_path = paths_to_search.pop(0)
        
        if not current_path.exists():
            continue
        
        for item in current_path.iterdir():
            if item.is_dir():
                paths_to_search.append(item)
            elif item.is_file():
                all_files.append(item)
    
    # Determine number of worker processes
    num_jobs = args.jobs
    total_files = len(all_files)
    
    print(f"Compressing textures using {num_jobs} parallel jobs")
    print(f"Total files: {total_files}\n")
    
    # Track active files and progress
    active_files = []
    active_lock = threading.Lock()
    completed = 0
    last_display_lines = 0
    
    def clear_display():
        if CI_MODE:
            return
        nonlocal last_display_lines
        for _ in range(last_display_lines):
            print("\r\033[K\033[A", end="")
        print("\r\033[K", end="")
    
    def update_display():
        """Update the progress display"""
        nonlocal last_display_lines
        
        lines = []
        
        # Show active files (up to 3)
        if active_files:
            lines.append("Currently processing:")
            for i, f in enumerate(active_files[:3]):
                lines.append(f"  {BULLET} {f}")
            if len(active_files) > 3:
                lines.append(f"  ... and {len(active_files) - 3} more")
            lines.append("")
        
        # Show progress bar
        progress = completed / total_files if total_files > 0 else 0
        bar_width = 50
        filled = int(bar_width * progress)
        bar = BAR_FULL * filled + BAR_EMPTY * (bar_width - filled)
        lines.append(f"[{bar}] {completed}/{total_files} ({progress*100:.1f}%)")
        
        # Print all lines
        for line in lines:
            print(line)
        
        last_display_lines = len(lines)
    
    def progress_callback(filename, status, elapsed=None):
        nonlocal completed, files_processed

        with active_lock:
            if status == "compressing":
                active_files.append(filename)
            elif status in ["completed", "skipped", "failed", "copied", "copy_failed"]:
                completed += 1
                if status in ["completed", "skipped", "copied"]:
                    files_processed += 1
                if filename in active_files:
                    active_files.remove(filename)

            clear_display()
            update_display()
    
    # Process files in parallel using threads
    with ThreadPoolExecutor(max_workers=num_jobs) as executor:
        # Submit all jobs
        future_to_file = {
            executor.submit(handle_file, file_path, source_path, destination_path, args, progress_callback, log_error): file_path
            for file_path in all_files
        }
        
        # Process completed jobs
        for future in as_completed(future_to_file):
            file_path = future_to_file[future]
            try:
                future.result()
            except Exception as e:
                log_error(f"Unhandled exception processing {file_path.name}: {str(e)}")
                with active_lock:
                    clear_display()
                    print(f"{Fore.RED}Error processing {file_path.name} {Style.RESET_ALL}(logged to {log_file.name})")
                    update_display()
    
    # Clear the final progress display
    with active_lock:
        clear_display()
        print(f"{Fore.GREEN}{CHECK} Completed: {files_processed}/{total_files} files processed {Style.RESET_ALL}")
        if error_count > 0:
            print(f"{Fore.RED}{WARN} {error_count} error(s) occurred. Check log file: {Style.RESET_ALL}{log_file}")
        print()
    
    elapsed = time.time() - start_time
    minutes = int(elapsed // 60)
    seconds = int(elapsed % 60)
    milliseconds = int((elapsed % 1) * 1000)
    
    print(f"Processed {files_processed} files in {minutes}m:{seconds}s:{milliseconds}ms")

if __name__ == "__main__":
    if CI_MODE:
        colorama_init(strip=True, convert=False)
    else:
        colorama_init()
    if not COMPRESSONATOR_CLI.exists():
        print(
            f"{Fore.RED}Error:{Style.RESET_ALL} "
            f"Compressonator CLI not found at:\n  {COMPRESSONATOR_CLI}"
        )
        sys.exit(1)
    main()