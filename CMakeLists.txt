cmake_minimum_required(VERSION 3.14)

project(gameboy)

set(SOURCE
	# entry point
	src/main.c

	# cpu
	src/core/cpu.c include/core/cpu.h

	# dmg
	src/core/dmg.c include/core/dmg.h

	# mmu
	src/core/mmu.c include/core/mmu.h

	# opc
	src/core/opc.c include/core/opc.h

	# ppu
	src/core/ppu.c include/core/ppu.h

	# rom
	src/core/rom.c include/core/rom.h
)

add_executable(gameboy ${SOURCE})

target_include_directories(gameboy PRIVATE include)
