cmake_minimum_required(VERSION 3.14)

project(gameboy)

set(SOURCE
	# entry point
	src/main.c

	# dmg
	src/core/dmg.c include/core/dmg.h

	# cpu
	src/core/cpu.c include/core/cpu.h

	# mmu
	src/core/mmu.c include/core/mmu.h

	# ppu
	src/core/ppu.c include/core/ppu.h
)

add_executable(gameboy ${SOURCE})

target_include_directories(gameboy PRIVATE include)
