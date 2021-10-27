#include "core/mmu.h"

#include <stdio.h>
#include <stdlib.h>

void mmu_create(mmu_t* mmu, rom_t* rom)
{
	mmu->cart[0] = &rom->data[0x0000];
	mmu->cart[1] = &rom->data[0x4000];
}

void mmu_destroy(mmu_t* mmu)
{
	
}

uint8_t* mmu_map(mmu_t* mmu, uint16_t address)
{
	switch (address & 0xF000)
	{
	case 0x0000:
	case 0x1000:
	case 0x2000:
	case 0x3000:
		return &mmu->cart[0][address];
	case 0x4000:
	case 0x5000:
	case 0x6000:
	case 0x7000:
		return &mmu->cart[1][address - 0x4000];
	case 0x8000:
	case 0x9000:
		return &mmu->vram[address - 0x8000];
	case 0xA000:
	case 0xB000:
		return &mmu->xram[address - 0xA000];
	case 0xC000:
		return &mmu->wram[0][address - 0xC000];
	case 0xD000:
		return &mmu->wram[1][address - 0xD000];
	default:
		printf("[!] unable to map address `0x%04X` to mmu", address);
		exit(EXIT_FAILURE);
	}
}

uint8_t mmu_peek8(mmu_t* mmu, uint16_t address)
{
	return *mmu_map(mmu, address);
}

uint16_t mmu_peek16(mmu_t* mmu, uint16_t address)
{
	return mmu_peek8(mmu, address + 1) << 8 | mmu_peek8(mmu, address);
}

void mmu_poke8(mmu_t* mmu, uint16_t address, uint8_t value)
{
	*mmu_map(mmu, address) = value;
}

void mmu_poke16(mmu_t* mmu, uint16_t address, uint16_t value)
{
	*mmu_map(mmu, address + 1) = value & 0xFF;
	*mmu_map(mmu, address) = (value >> 8) & 0xFF;
}
