cmake_minimum_required(VERSION 3.14)

project(client CXX)
set(CMAKE_CXX_STANDARD 14)

set(SOURCE
    src/main.cpp
    src/audio.cpp include/audio.hpp
    src/window.cpp include/window.hpp)

add_subdirectory(deps/sdl2)
add_subdirectory(deps/sfml)

add_executable(client ${SOURCE})

target_include_directories(client PRIVATE include deps/sdl2/include deps/sfml/include)
target_link_libraries(client sfml-graphics SDL2-static)

target_compile_options(client PRIVATE -Wno-switch)
