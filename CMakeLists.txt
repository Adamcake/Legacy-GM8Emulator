cmake_minimum_required(VERSION 3.7)
project(GM8Emulator)

set(CMAKE_CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)

include("deps/cmake/cmake-precompiled-header.cmake")

set(AMD64 OFF CACHE BOOL "" FORCE) # zlib
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(deps/glfw)
add_subdirectory(deps/zlib)
add_subdirectory(src)

set_target_properties(example PROPERTIES FOLDER "zlib")
set_target_properties(minigzip PROPERTIES FOLDER "zlib")
set_target_properties(zlib PROPERTIES FOLDER "zlib")
set_target_properties(zlibstatic PROPERTIES FOLDER "zlib")
