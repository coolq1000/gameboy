cmake_minimum_required(VERSION 3.14)

project(mojoal)

set(SOURCE
    src/mojoal.c
    include/al/al.h include/al/alc.h
)

add_library(mojoal STATIC ${SOURCE})

target_include_directories(mojoal PUBLIC include)
