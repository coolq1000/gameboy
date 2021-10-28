#include "core/dmg.h"

void dmg_create(dmg_t* dmg, rom_t* rom)
{
	cpu_create(&dmg->cpu);
	mmu_create(&dmg->mmu, rom);
	ppu_create(&dmg->ppu);
}

void dmg_destroy(dmg_t* dmg)
{
	cpu_destroy(&dmg->cpu);
	mmu_destroy(&dmg->mmu);
	ppu_destroy(&dmg->ppu);
}

void dmg_cycle(dmg_t* dmg)
{
	cpu_cycle(&dmg->cpu, &dmg->mmu);
}
