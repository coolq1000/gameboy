cmake_minimum_required(VERSION 3.14)

project(core)

set(SOURCE
	# cpu
	src/cpu.c include/core/cpu.h

	# dmg
	src/dmg.c include/core/dmg.h

	# mmu
	src/mmu.c include/core/mmu.h

	# opc
	src/opc.c include/core/opc.h

	# ppu
	src/ppu.c include/core/ppu.h

	# rom
	src/rom.c include/core/rom.h
)

add_library(core STATIC ${SOURCE})

target_include_directories(core PUBLIC include)
