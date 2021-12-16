cmake_minimum_required(VERSION 3.14)

project(client CXX)
set(CMAKE_CXX_STANDARD 14)

set(SOURCE
	src/main.cpp
	src/audio.cpp include/audio.hpp
)

set(BUILD_SHARED_LIBS FALSE)
if (APPLE)
	# set(SFML_INSTALL_XCODE_TEMPLATES TRUE)
	set(CMAKE_WARN_DEPRECATED FALSE)
	set(BUILD_SHARED_LIBS TRUE)
endif()
add_subdirectory(deps/sfml)

add_executable(client ${SOURCE})

target_include_directories(client PRIVATE include)

target_include_directories(client PRIVATE include deps/sfml/include)
target_link_libraries(client sfml-graphics sfml-audio)

target_compile_options(client PRIVATE -Wno-switch)

if (WIN32)
	if (CMAKE_SIZEOF_VOID_P EQUAL 8)
		configure_file(deps/sfml/extlibs/bin/x64/openal32.dll ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
	else ()
		configure_file(deps/sfml/extlibs/bin/x86/openal32.dll ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
	endif ()
endif ()
