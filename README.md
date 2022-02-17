# RCHAC

Reality Core hardware accelerated video encoder and decoder. This API uses the Vulkan's video encoding/ decoding framework to encode or decode videos to H.265, frame by frame.

## How to build

To build the project, you need the [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) and the [NVIDIA beta drivers](https://developer.nvidia.com/vulkan-driver).
After installing the Vulkan SDK, make sure that the `vulkan_sdk` environment variable is properly set to the install location.

After setting up the dependencies, you can proceed to clone the repository.

```bash
git clone https://github.com/Intro-Ventors/RCHAC {SOURCE}
cd {SOURCE}
```

*Here `{SOURCE}` is the place where the repository is cloned to.*

After cloning it, run the `Bootstrap.bat` file (if on Windows) or `Bootstrap.sh` (if on Linux) to build the required files. On Windows, it will build the required `.sln` and `.vcxproj` files which you can then open using Visual Studio, or on Linux, it will generate the required `gmake2` files.

Once this is done, you can proceed to build it via either Visual Studio (if on Windows) or Makefile (if on Linux).

> Note that to properly run the application built using this library, it requires the Vulkan shared library.

## License

This repository is licensed under MIT.
