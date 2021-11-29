#include "core/dmg.h"

void dmg_create(dmg_t* dmg, rom_t* rom, bool is_cgb)
{
	cpu_create(&dmg->cpu, is_cgb);
	mmu_create(&dmg->mmu, rom);
	ppu_create(&dmg->ppu, is_cgb);
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
	ppu_cycle(&dmg->ppu, &dmg->mmu, &dmg->cpu, dmg->cpu.clock.cycles);
}
