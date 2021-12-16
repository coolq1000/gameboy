
#ifndef DMG_H
#define DMG_H

#include "cpu.h"
#include "mmu.h"
#include "ppu.h"

typedef struct dmg_t
{
	cpu_t cpu;
	mmu_t mmu;
	ppu_t ppu;
} dmg_t;

void dmg_create(dmg_t* dmg, rom_t* rom, bool is_cgb);
void dmg_destroy(dmg_t* dmg);

void dmg_cycle(dmg_t* dmg);

#endif
