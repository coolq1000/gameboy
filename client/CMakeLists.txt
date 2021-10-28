cmake_minimum_required(VERSION 3.14)

project(client)

set(SOURCE
	src/main.cpp
)

add_executable(client ${SOURCE})

target_include_directories(client PRIVATE include)
