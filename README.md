# GM8Emulator
An emulator for games created with GameMaker 8. This repo is no longer being developed and has been marked as read-only. The project has been broadened to "OpenGM8" and a more advanced version of this application, written in Rust, can be found [here](https://github.com/OpenGM8/GM8Emulator). You can build this if you want, but expect crashes, missing features and other noticeable issues.

## Building
- Clone the repository, make sure to grab submodules along with it (`--recurse-submodules` or `submodule update --init --recursive`)
- If you want to use CMake, run the CMakeLists file in the repository root, that's all the setup needed
- For building with plain GCC/MinGW/MinGW-w64:
  - Sources: `./src/*.cpp` `./deps/glad/src/glad.c`
  - Include: `./src/` `./deps/glfw/include/` `./deps/zlib/` `./deps/rectpack2D/src/` `./deps/glad/include/`
  - Libraries: `-lz` `-lglfw3` (and `-lgdi32` `-lopengl32` if you're on Windows, should come with MinGW)
  - Make sure to build with `--std=c++17` and `-Ofast`

## Contact
gm8emulator@gmail.com
