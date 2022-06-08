cmake_minimum_required(VERSION 3.14)

project(gameboy)

add_subdirectory(core)

set(CMAKE_CXX_STANDARD 20)

set(SOURCE
    src/main.cpp
    src/audio.cpp include/audio.hpp
    src/window.cpp include/window.hpp)

set(SDL_STATIC TRUE)
add_subdirectory(deps/sdl2)

add_executable(gameboy ${SOURCE})

target_include_directories(gameboy PRIVATE include deps/sdl2/include)
target_link_libraries(gameboy SDL2-static)

#target_compile_options(client PRIVATE -Wno-switch)
#target_link_options(client PRIVATE "-mconsole")

target_link_libraries(gameboy core)
