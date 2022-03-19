"""
Build script for Firefly.

This script is responsible for downloading additional dependencies and setting up the third party libraries required by the library.
"""

import os
import sys


def setup_windows():
    # Setup GLFW.
    if os.path.exists("Include\\GLFW\\build"):
        os.system("cd Include\\GLFW && cmake -S . -B build && cd build && cmake --build . --config Release && cmake --build . --config Debug")
    else:
        os.system("cd Include\\GLFW && mkdir -p build && cmake -S . -B build && cd build && cmake --build . --config Release && cmake --build . -- config Debug")

    # Build the required project files.
    os.system("call Tools\Windows\premake5 vs2022")


def setup_linux():
    # Setup GLFW.
    if os.path.exists("Include/GLFW/build"):
        os.system("cd Include/GLFW && cmake -S . -B build && cd build && cmake --build . --config Release && cmake --build . --config Debug")
    else:
        os.system("cd 'Include/GLFW' && mkdir -p build && cmake -S . -B build && cd build && cmake --build . --config Release && cmake --build . --config Debug")

    # Build the required project files.
    os.system("call Tools\Linux\premake5 gmake2")


def setup_macos():
    # Setup GLFW.
    if os.path.exists("Include/GLFW/build"):
        os.system("cd Include/GLFW && cmake -S . -B build && cd build && cmake --build . --config Release && cmake --build . --config Debug")
    else:
        os.system("cd 'Include/GLFW' && mkdir -p build && cmake -S . -B build && cd build && cmake --build . --config Release && cmake --build . --config Debug")

    # Build the required project files.
    os.system("call Tools\Mac\premake5 xcode4")


if __name__ == "__main__":
    if sys.platform == "win32":
        setup_windows();

    elif sys.platform.startswith("linux"):
        setup_linux()

    elif sys.platform == "darwin":
        setup_macos()

    else:
        print("Unsupported platform!")
