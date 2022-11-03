#include "core/dmg.h"

void dmg_init(dmg_t *dmg, rom_t *rom, bool is_cgb, usize sample_rate, usize latency)
{
    /* initialize components */
    apu_init(&dmg->apu, sample_rate, latency);
    cpu_init(&dmg->cpu, is_cgb);
    mmu_init(&dmg->mmu, rom);
    ppu_init(&dmg->ppu, is_cgb);

    /* map memory components onto bus */
    bus_init(&dmg->bus, &dmg->cpu, &dmg->apu, &dmg->mmu, &dmg->ppu);
}

void dmg_free(dmg_t *dmg)
{
    mmu_free(&dmg->mmu);
}

void dmg_cycle(dmg_t *dmg)
{
    cpu_cycle(&dmg->cpu, &dmg->bus);
    ppu_cycle(&dmg->ppu, &dmg->bus, dmg->cpu.clock.cycles);
    apu_cycle(&dmg->apu, &dmg->bus, dmg->cpu.clock.cycles);
}
