#!/usr/bin/env python3

import argparse
import socket
import subprocess
import sys
import time
import webbrowser
from pathlib import Path


PORT = 8000


def is_port_in_use(port: int) -> bool:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        return sock.connect_ex(("localhost", port)) == 0


def start_server(directory: Path, port: int):
    print(f"Starting web server on port {port}...")

    subprocess.Popen(
        [
            sys.executable,
            "-m",
            "http.server",
            str(port),
            "--directory",
            str(directory),
        ],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
        start_new_session=True,
    )


def main():
    parser = argparse.ArgumentParser(
        description="Launch a local web server for an Emscripten build."
    )
    parser.add_argument(
        "build_directory",
        type=Path,
        help="Directory containing the generated HTML/JS/WASM files",
    )

    args = parser.parse_args()

    build_dir = args.build_directory.resolve()

    if not build_dir.exists():
        print(f"Error: directory does not exist: {build_dir}")
        return 1

    if not is_port_in_use(PORT):
        start_server(build_dir, PORT)
    else:
        print(f"Web server already running on port {PORT}.")

    # Give the server time to start
    time.sleep(1)

    url = f"http://localhost:{PORT}"
    print(f"Opening {url}")
    webbrowser.open(url)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())