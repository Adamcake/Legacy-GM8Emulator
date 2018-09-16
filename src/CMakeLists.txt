# it uses globs, remake on pull. it's not that hard.
file(GLOB Source "*.cpp")
file(GLOB Headers "*.hpp")

add_executable(GM8Emulator ${Source} ${Headers})
source_group("Headers" FILES ${Headers})
source_group("Source" FILES ${Source})

# Don't build tests, examples or documentation for GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

include_directories("../glfw/include")
target_link_libraries(GM8Emulator glfw)

find_package(OpenGL REQUIRED)
if(OPENGL_FOUND)
    target_include_directories(GM8Emulator PUBLIC ${OPENGL_INCLUDE_DIR})
    target_link_libraries(GM8Emulator ${OPENGL_gl_LIBRARY})
else()
    message(FATAL_ERROR "Could not find OpenGL!")
endif()

if(MSVC)
    target_include_directories(GM8Emulator PUBLIC "../vs_deps/glew/include")
    target_include_directories(GM8Emulator PUBLIC "../vs_deps/zlib/include")

    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        target_link_libraries(GM8Emulator "../vs_deps/zlib/lib/zlib64")
        target_link_libraries(GM8Emulator "../vs_deps/glew/lib/glew64")
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        target_link_libraries(GM8Emulator "../vs_deps/zlib/lib/zlib")
        target_link_libraries(GM8Emulator "../vs_deps/glew/lib/glew")
    endif()
else()
    find_package(GLEW REQUIRED)
    if(GLEW_FOUND)
        include_directories(${ZLIB_INCLUDE_DIRS})
        target_link_libraries(GM8Emulator ${ZLIB_LIBRARIES})
    endif()

    find_package(ZLIB REQUIRED)
    if(ZLIB_FOUND)
        include_directories(${ZLIB_INCLUDE_DIRS})
        target_link_libraries(GM8Emulator ${ZLIB_LIBRARIES})
    endif()
endif()
