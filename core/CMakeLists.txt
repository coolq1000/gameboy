cmake_minimum_required(VERSION 3.14)

project(core)

set(SOURCE
	src/apu.c include/core/apu.h
	src/cpu.c include/core/cpu.h
	src/dmg.c include/core/dmg.h
	src/mmu.c include/core/mmu.h
	src/opc.c include/core/opc.h
	src/ppu.c include/core/ppu.h
	src/rom.c include/core/rom.h
)

add_library(core STATIC ${SOURCE})

target_include_directories(core PUBLIC include)
