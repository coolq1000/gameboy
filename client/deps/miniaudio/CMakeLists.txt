cmake_minimum_required(VERSION 3.14)

project(miniaudio)

set(SOURCE
    src/miniaudio.c include/miniaudio/miniaudio.h
)

add_library(miniaudio STATIC ${SOURCE})

target_include_directories(miniaudio PUBLIC include)
