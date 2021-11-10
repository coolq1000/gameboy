#include "core/mmu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void mmu_create(mmu_t* mmu, rom_t* rom)
{
	/* point cartridge memory to rom data */
	mmu->cart[0] = &rom->cart_data[0x0000];
	mmu->cart[1] = &rom->cart_data[0x4000];

	/* allocate memory */
	mmu->vram = (uint8_t*)malloc(0x2000);
	for (size_t i = 0; i < MBC5_XRAM_COUNT; i++)
		mmu->xram[i] = (uint8_t*)malloc(XRAM_SIZE);
	for (size_t i = 0; i < CGB_WRAM_COUNT; i++)
		mmu->wram[i] = (uint8_t*)malloc(WRAM_SIZE);
	mmu->oam = (uint8_t*)malloc(OAM_SIZE);
	mmu->io = (uint8_t*)malloc(IO_SIZE);
	mmu->hram = (uint8_t*)malloc(HRAM_SIZE);

	/* for null access in memory map */
	mmu->null_mem = 0;

	/* clear out memory */
	memset(mmu->vram, 0, sizeof(mmu->vram));
	for (size_t i = 0; i < MBC5_XRAM_COUNT; i++)
		memset(mmu->xram[i], 0, sizeof(mmu->xram));
	for (size_t i = 0; i < CGB_WRAM_COUNT; i++)
		memset(mmu->wram[i], 0, sizeof(mmu->wram[0]));
	memset(mmu->oam, 0, sizeof(mmu->oam));
	memset(mmu->io, 0, sizeof(mmu->io));
	memset(mmu->hram, 0, sizeof(mmu->hram));
	mmu->interrupt_enable = 0;

	/* setup memory */
	mmu->io[MMAP_IO_LCDC - 0xFF00] = 0x91; // LCDC

	/* load save data */
	if (rom->save_data)
	{
		memcpy(mmu->xram[0], rom->save_data, XRAM_SIZE); // todo read all buffers
	}
}

void mmu_destroy(mmu_t* mmu)
{
	free(mmu->vram);
	for (size_t i = 0; i < MBC5_XRAM_COUNT; i++)
		free(mmu->xram[i]);
	for (size_t i = 0; i < CGB_WRAM_COUNT; i++)
		free(mmu->wram[i]);
	free(mmu->oam);
	free(mmu->io);
	free(mmu->hram);
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
		return &mmu->xram[0][address - 0xA000];
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
			return address < 0xFEA0 ? &mmu->oam[address - 0xFE00] : &mmu->null_mem;
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
				{
					uint8_t original = mmu->io[0x00] & 0x30;
					uint8_t input = 0b11000000;
					if (!(mmu->io[0x00] & 0x10)) /* directions */
					{
						input |= (mmu->buttons.right	? 0 : 0b00000001);
						input |= (mmu->buttons.left		? 0 : 0b00000010);
						input |= (mmu->buttons.up		? 0 : 0b00000100);
						input |= (mmu->buttons.down		? 0 : 0b00001000);
					}
					else if (!(mmu->io[0x00] & 0x20)) /* actions */
					{
						input |= (mmu->buttons.a		? 0 : 0b00000001);
						input |= (mmu->buttons.b		? 0 : 0b00000010);
						input |= (mmu->buttons.select	? 0 : 0b00000100);
						input |= (mmu->buttons.start	? 0 : 0b00001000);
					}
					else
					{
						input |= 0b00001111;
					}
					mmu->io[0x00] = input | original;
					return &mmu->io[address - 0xFF00];
				}
				case 0x10: case 0x20: case 0x30:
				case 0x40: case 0x50: case 0x60:
				case 0x70:
					return &mmu->io[address - 0xFF00];
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
	switch (address)
	{
	case 0xFF41:
		return mmu->io[MMAP_IO_STAT & 0xFF] | 0x80;
	}
	return *mmu_map(mmu, address);
}

uint16_t mmu_peek16(mmu_t* mmu, uint16_t address)
{
	return mmu_peek8(mmu, address + 1) << 8 | mmu_peek8(mmu, address);
}

void mmu_poke8(mmu_t* mmu, uint16_t address, uint8_t value)
{
	if (address >= 0x8000) // disallow writing to rom
	{
		switch (address)
		{
		case 0xFF00:
		{
			uint8_t input = mmu->io[0x00];
			mmu->io[0x00] = (value & 0x30) | (input & 0xCF); // only permit writing to bits 4 & 5
			return;
		}
		case 0xFF46: /* dma transfer */
			for (uint16_t copy_addr = value << 8; (copy_addr & 0xFF) < 0x9F; copy_addr++)
			{
				mmu_poke8(mmu, 0xFE00 + (copy_addr & 0xFF), mmu_peek8(mmu, copy_addr));
			}
			return;
		}
		*mmu_map(mmu, address) = value;
	}
	else
	{
		/* mbc5 implementation */
		switch (address & 0xF000)
		{
		case 0x0000:
		case 0x1000:
			/* ram enable */
			// todo: enable ram
			break;
		case 0x2000:
			mmu->cart[1] = mmu->cart[0] + (0x4000 * (value ? value : 1)); // todo: check this should go up to 1
			break;
		case 0x4000:
			break; // todo: fixme!!!
		case 0x6000:
			break; // todo: latch rtc timer!
		default:
			printf("attempt to write to ROM: %X = %X\n", address, value);
			exit(EXIT_FAILURE);
		}
	}
}

void mmu_poke16(mmu_t* mmu, uint16_t address, uint16_t value)
{
	mmu_poke8(mmu, address, value & 0xFF);
	mmu_poke8(mmu, address + 1, (value >> 8) & 0xFF);
}
