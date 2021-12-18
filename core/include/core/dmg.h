
#ifndef DMG_H
#define DMG_H

#include "cpu.h"
#include "bus.h"

typedef struct
{
    apu_t apu;
	cpu_t cpu;
	mmu_t mmu;
	ppu_t ppu;
    bus_t bus;
} dmg_t;

void dmg_create(dmg_t* dmg, rom_t* rom, bool is_cgb);
void dmg_destroy(dmg_t* dmg);

void dmg_cycle(dmg_t* dmg);

#endif
