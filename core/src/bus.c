#include "core/bus.h"

#include <stdio.h>
#include <stdlib.h>
#include "core/apu.h"
#include "core/mmu.h"
#include "core/ppu.h"

void bus_init(bus_t* bus, cpu_t* cpu, apu_t* apu, mmu_t* mmu, ppu_t* ppu)
{
    bus->cpu = cpu;
    bus->apu = apu;
    bus->mmu = mmu;
    bus->ppu = ppu;
}

u8 bus_peek8(bus_t* bus, u16 address)
{
    /* apu memory map */
    if (address >= MMAP_IO_NR10 && address <= MMAP_IO_NR52)
    {
        return apu_peek(bus->apu, address);
    }

    /* default back to mmu */
    return mmu_peek(bus->mmu, address);
}

u16 bus_peek16(bus_t* bus, u16 address)
{
    return mmu_peek(bus->mmu, address + 1) << 8 | mmu_peek(bus->mmu, address);
}

void bus_poke8(bus_t* bus, u16 address, u8 value)
{
    /* apu memory map */
    if (address >= MMAP_IO_NR10 && address <= MMAP_IO_NR52)
    {
        apu_poke(bus->apu, address, value);
        return;
    }

    /* default back to mmu */
    mmu_poke(bus->mmu, address, value);
}

void bus_poke16(bus_t* bus, u16 address, u16 value)
{
    mmu_poke(bus->mmu, address, value & 0xFF);
    mmu_poke(bus->mmu, address + 1, (value >> 8) & 0xFF);
}
