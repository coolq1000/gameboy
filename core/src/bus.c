#include "core/bus.h"

#include "core/apu.h"
#include "core/mmu.h"
#include "core/ppu.h"

void bus_init(bus_t* bus, apu_t* apu, mmu_t* mmu, ppu_t* ppu)
{
    bus->apu = apu;
    bus->mmu = mmu;
    bus->ppu = ppu;
}

u8 bus_peek8(bus_t* bus, u16 address)
{
    return mmu_peek(bus->mmu, address);
}

u16 bus_peek16(bus_t* bus, u16 address)
{
    return mmu_peek(bus->mmu, address + 1) << 8 | mmu_peek(bus->mmu, address);
}

void bus_poke8(bus_t* bus, u16 address, u8 value)
{
    mmu_poke(bus->mmu, address, value);
}

void bus_poke16(bus_t* bus, u16 address, u16 value)
{
    mmu_poke(bus->mmu, address, value & 0xFF);
    mmu_poke(bus->mmu, address + 1, (value >> 8) & 0xFF);
}
