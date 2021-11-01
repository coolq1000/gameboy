#include "core/mmu.h"

#include <stdio.h>
#include <stdlib.h>

void mmu_create(mmu_t* mmu, rom_t* rom)
{
	mmu->cart[0] = &rom->data[0x0000];
	mmu->cart[1] = &rom->data[0x4000];

	mmu->null_mem = 0;

	/* clear out memory */
	for (uint16_t address = 0x8000; address < 0xFFFE; address++) mmu_poke8(mmu, address, 0x00);
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
	case 0xE000:
		return &mmu->wram[0][address - 0xE000];
	case 0xF000:
		switch (address & 0xF00)
		{
		case 0x000: case 0x100: case 0x200: case 0x300:
		case 0x400: case 0x500: case 0x600: case 0x700:
		case 0x800: case 0x900: case 0xA00: case 0xB00:
		case 0xC00: case 0xD00:
			return &mmu->wram[0][address - 0xF000];
		case 0xE00:
			return address < 0xFEA0 ? &mmu->oam[address & 0xFF] : &mmu->null_mem;
		case 0xF00:
			if (address == 0xFFFF)
			{
				return &mmu->interrupt_enable;
			}
			else
			{
				switch (address & 0xF0)
				{
				case 0x00:
					mmu->io[0x00] = 0xEF;
					return &mmu->io[address & 0xFF];
				case 0x10: case 0x20: case 0x30:
				case 0x40: case 0x50: case 0x60:
				case 0x70:
					return &mmu->io[address & 0xFF];
				}
				return &mmu->hram[address - 0xFF80];
			}
		}
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
	// if (address >= 0x8000 && address < 0xA000)
	// {
	// 	printf("wrote vram: %X = %X\n", address, value);
	// }
	*mmu_map(mmu, address) = value;
}

void mmu_poke16(mmu_t* mmu, uint16_t address, uint16_t value)
{
	mmu_poke8(mmu, address, value & 0xFF);
	mmu_poke8(mmu, address + 1, (value >> 8) & 0xFF);
}
