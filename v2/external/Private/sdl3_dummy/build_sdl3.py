#!/usr/bin/env python3
import subprocess, sys, os

cfg = sys.argv[1]           # e.g. "Debug"
system = sys.argv[2]        # e.g. "emscripten", "windows", "linux", "macosx"

script_dir = os.path.dirname(os.path.abspath(__file__))
src = os.path.abspath(os.path.join(script_dir, "..", "sdl3"))
build_dir = os.path.join(src, f"build-{system}")

sdl_flags = [
    "-DSDL_SHARED=OFF", "-DSDL_STATIC=ON", "-DSDL_GPU=OFF", "-DSDL_RENDER=OFF",
    "-DSDL_AUDIO=OFF", "-DSDL_CAMERA=OFF", "-DSDL_DISKAUDIO=OFF", "-DSDL_DUMMYAUDIO=OFF",
    "-DSDL_TESTS=OFF", "-DSDL_EXAMPLES=OFF", "-DSDL_INSTALL=OFF",
    "-DSDL_INSTALL_TESTS=OFF", "-DSDL_TEST_LIBRARY=OFF",
]

if system == "emscripten":
    configure = [
        "emcmake", "cmake",
        "-S", src,
        "-B", build_dir,
        "-G", "Ninja",
        f"-DCMAKE_BUILD_TYPE={cfg}",
        "-DCMAKE_C_FLAGS=-m64",
        "-DCMAKE_CXX_FLAGS=-m64",
    ] + sdl_flags
else:
    configure = ["cmake", "-S", src, "-B", build_dir,
                 f"-DCMAKE_BUILD_TYPE={cfg}"] + sdl_flags

subprocess.run(configure, check=True)
subprocess.run(["cmake", "--build", build_dir, "--config", cfg], check=True)