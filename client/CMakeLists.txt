cmake_minimum_required(VERSION 3.14)

project(client CXX)
set(CMAKE_CXX_STANDARD 14)

set(SOURCE
    src/main.cpp
    src/audio.cpp include/audio.hpp
    src/window.cpp include/window.hpp)

set(SDL_STATIC TRUE)
add_subdirectory(deps/sdl2)

add_executable(client ${SOURCE})

target_include_directories(client PRIVATE include deps/sdl2/include)
target_link_libraries(client SDL2-static)

#target_compile_options(client PRIVATE -Wno-switch)
#target_link_options(client PRIVATE "-mconsole")
