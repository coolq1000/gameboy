
#ifndef BUS_H
#define BUS_H

#include "util.h"

typedef struct apu apu_t;
typedef struct mmu mmu_t;
typedef struct ppu ppu_t;

typedef struct bus
{
    apu_t* apu;
    mmu_t* mmu;
    ppu_t* ppu;
} bus_t;

void bus_init(bus_t* bus, apu_t* apu, mmu_t* mmu, ppu_t* ppu);

u8 bus_peek8(bus_t* bus, u16 address);
u16 bus_peek16(bus_t* bus, u16 address);

void bus_poke8(bus_t* bus, u16 address, u8 value);
void bus_poke16(bus_t* bus, u16 address, u16 value);

#endif
