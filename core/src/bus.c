#include "core/bus.h"

void bus_init(bus_t* bus, apu_t* apu, mmu_t* mmu, ppu_t* ppu)
{
    bus->apu = apu;
    bus->mmu = mmu;
    bus->ppu = ppu;
}

u8 bus_peek8(bus_t* bus, u16 address)
{
    return mmu_peek8(bus->mmu, address);
}

u16 bus_peek16(bus_t* bus, u16 address)
{
    return mmu_peek16(bus->mmu, address);
}

void bus_poke8(bus_t* bus, u16 address, u8 value)
{
    mmu_poke8(bus->mmu, address, value);
}

void bus_poke16(bus_t* bus, u16 address, u16 value)
{
    mmu_poke16(bus->mmu, address, value);
}
