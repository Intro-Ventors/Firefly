# Firefly

Firefly is a low level graphics API supporting multiple graphics operations (depending on the platform) using Vulkan. It currently supports Windows, Linux, iOS and Android.

## Dependencies

This library has minimal dependencies, but it requires the [CMake](https://cmake.org/) build system and the [Python](https://www.python.org/) language interpreter to generate the required libraries. 
But if you are on Linux, please download the required packages as requested by [GLFW](https://www.glfw.org/docs/latest/compile.html).

## How to use

First, clone this repository.

```bash
git clone https://github.com/Intro-Ventors/Firefly {SOURCE}
cd {SOURCE}
```

*Here `{SOURCE}` is the place where the repository is cloned to.*

After cloning it, run the `Bootstrap.bat` file (if on Windows) or `Bootstrap.sh` (if on Linux) to build the required files. On Windows, it will build the required `.sln` and `.vcxproj` 
files which you can then open using Visual Studio, or on Linux, it will generate the required `gmake2` files.

Hereafter, there are two ways to use this library, either compile all the source files under `{SOURCE}/Include/Firefly/Source` or just include the `ImportSourceFiles.hpp` file which is under
`{SOURCE}/Include/Firefly`. This file should be included in exactly only ONE source file. We also require you to link against the `glfw3dll` static library and to also copy the `glfw3` shared 
library to the executable directory. These library files will be located under `Include/GLFW/build/src/{CONFIG}`.

> Note that to properly run the application built using this library, it requires the [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/).

## License

This repository is licensed under MIT.
