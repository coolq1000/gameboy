
#ifndef DMG_H
#define DMG_H

#include "apu.h"
#include "cpu.h"
#include "mmu.h"
#include "ppu.h"
#include "bus.h"

typedef struct dmg
{
    apu_t apu;
	cpu_t cpu;
	mmu_t mmu;
	ppu_t ppu;
    bus_t bus;
} dmg_t;

void dmg_init(dmg_t* dmg, rom_t* rom, bool is_cgb);
void dmg_free(dmg_t* dmg);

void dmg_cycle(dmg_t* dmg);

#endif
