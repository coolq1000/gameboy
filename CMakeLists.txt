cmake_minimum_required(VERSION 3.14)

project(gameboy)

add_subdirectory(core)
add_subdirectory(client)

target_link_libraries(client core)
