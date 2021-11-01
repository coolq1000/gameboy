cmake_minimum_required(VERSION 3.14)

project(client)

set(SOURCE
	src/main.cpp
)

set(BUILD_SHARED_LIBS FALSE)
add_subdirectory(deps/sfml)

add_executable(client ${SOURCE})

target_include_directories(client PRIVATE include)

target_include_directories(client PRIVATE deps/sfml/include)
target_link_libraries(client sfml-graphics sfml-audio ${SFML_LIBRARIES})
