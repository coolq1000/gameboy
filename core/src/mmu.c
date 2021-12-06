#include "core/mmu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void mmu_create(mmu_t* mmu, rom_t* rom)
{
	/* point cartridge memory to rom data */
	mmu->memory.cart[0] = &rom->cart_data[MMAP_ROM_00];
	mmu->memory.cart[1] = &rom->cart_data[MMAP_ROM_01];

	/* allocate memory */
	for (size_t i = 0; i < CGB_VRAM_COUNT; i++)
		mmu->memory.vram[i] = (uint8_t*)malloc(VRAM_SIZE);
	for (size_t i = 0; i < MBC5_XRAM_COUNT; i++)
		mmu->memory.xram[i] = (uint8_t*)malloc(XRAM_SIZE);
	for (size_t i = 0; i < CGB_WRAM_COUNT; i++)
		mmu->memory.wram[i] = (uint8_t*)malloc(WRAM_SIZE);
	mmu->memory.oam = (uint8_t*)malloc(OAM_SIZE);
	mmu->memory.io = (uint8_t*)malloc(IO_SIZE);
	mmu->memory.hram = (uint8_t*)malloc(HRAM_SIZE);

	/* for null access in memory map */
	mmu->null_mem = 0;

	/* clear out memory */
	for (size_t i = 0; i < CGB_VRAM_COUNT; i++)
		memset(mmu->memory.vram[i], 0, VRAM_SIZE);
	for (size_t i = 0; i < MBC5_XRAM_COUNT; i++)
		memset(mmu->memory.xram[i], 0, XRAM_SIZE);
	for (size_t i = 0; i < CGB_WRAM_COUNT; i++)
		memset(mmu->memory.wram[i], 0, WRAM_SIZE);
	memset(mmu->memory.oam, 0, sizeof(mmu->memory.oam));
	memset(mmu->memory.io, 0, sizeof(mmu->memory.io));
	memset(mmu->memory.hram, 0, sizeof(mmu->memory.hram));
	mmu->memory.interrupt_enable = 0;

	/* setup memory */
	mmu->io.lcdc = 0x91; // LCDC

	/* load save data */
	if (rom->save_data)
	{
		memcpy(mmu->memory.xram[0], rom->save_data, XRAM_SIZE); // todo read all buffers
	}
}

void mmu_destroy(mmu_t* mmu)
{
	for (size_t i = 0; i < CGB_VRAM_COUNT; i++)
		free(mmu->memory.vram[i]);
	for (size_t i = 0; i < MBC5_XRAM_COUNT; i++)
		free(mmu->memory.xram[i]);
	for (size_t i = 0; i < CGB_WRAM_COUNT; i++)
		free(mmu->memory.wram[i]);
	free(mmu->memory.oam);
	free(mmu->memory.io);
	free(mmu->memory.hram);
}

uint8_t* mmu_map(mmu_t* mmu, uint16_t address)
{
	switch (address & 0xF000)
	{
	case 0x0000:
	case 0x1000:
	case 0x2000:
	case 0x3000:
		return &mmu->memory.cart[0][address];
	case 0x4000:
	case 0x5000:
	case 0x6000:
	case 0x7000:
		return &mmu->memory.cart[1][address - 0x4000];
	case 0x8000:
	case 0x9000:
		return &mmu->memory.vram[mmu->io.vbk][address - 0x8000];
	case 0xA000:
	case 0xB000:
		return &mmu->memory.xram[0][address - 0xA000];
	case 0xC000:
		return &mmu->memory.wram[0][address - 0xC000];
	case 0xD000:
		return &mmu->memory.wram[mmu->io.svbk ? mmu->io.svbk : 1][address - 0xD000];
	case 0xE000:
		return &mmu->memory.wram[0][address - 0xE000];
	case 0xF000:
		switch (address & 0xF00)
		{
		case 0x000: case 0x100: case 0x200: case 0x300:
		case 0x400: case 0x500: case 0x600: case 0x700:
		case 0x800: case 0x900: case 0xA00: case 0xB00:
		case 0xC00: case 0xD00:
			return &mmu->memory.wram[0][address - 0xF000];
		case 0xE00:
			return address < 0xFEA0 ? &mmu->memory.oam[address - 0xFE00] : &mmu->null_mem;
		case 0xF00:
			switch (address)
			{
			case MMAP_IO_DIV:
				return &mmu->io.div;
			case MMAP_IO_TIMA:
				return &mmu->io.tima;
			case MMAP_IO_TMA:
				return &mmu->io.tma;
			case MMAP_IO_TAC:
				return &mmu->io.tac;
			case MMAP_IO_IRF:
				return &mmu->io.irf;
			case MMAP_IO_LCDC:
				return &mmu->io.lcdc;
			case MMAP_IO_STAT:
				return &mmu->io.stat;
			case MMAP_IO_SCY:
				return &mmu->io.scy;
			case MMAP_IO_SCX:
				return &mmu->io.scx;
			case MMAP_IO_LY:
				return &mmu->io.ly;
			case MMAP_IO_LYC:
				return &mmu->io.lyc;
			case MMAP_IO_BGP:
				return &mmu->io.bgp;
			case MMAP_IO_OBP0:
				return &mmu->io.obp0;
			case MMAP_IO_OBP1:
				return &mmu->io.obp1;
			case MMAP_IO_WY:
				return &mmu->io.wy;
			case MMAP_IO_WX:
				return &mmu->io.wx;
			case MMAP_IO_VBK:
				return &mmu->io.vbk;
			case MMAP_IO_BGPI:
				return &mmu->io.bgpi;
			case MMAP_IO_BGPD:
				return &mmu->io.bgpd;
			case MMAP_IO_OBPI:
				return &mmu->io.obpi;
			case MMAP_IO_OBPD:
				return &mmu->io.obpd;
			case MMAP_IO_SVBK:
				return &mmu->io.svbk;
			case MMAP_IE:
				return &mmu->memory.interrupt_enable;
			default:
				switch (address & 0xF0)
				{
				case 0x00:
				{
					uint8_t original = mmu->io.joyp & 0x30;
					uint8_t input = 0b11000000;

					/* unpack buttons, so we can modify them */
					uint8_t right = mmu->buttons.right;
					uint8_t left = mmu->buttons.left;
					uint8_t up = mmu->buttons.up;
					uint8_t down = mmu->buttons.down;
					uint8_t a = mmu->buttons.a;
					uint8_t b = mmu->buttons.b;
					uint8_t select = mmu->buttons.select;
					uint8_t start = mmu->buttons.start;

					/* you couldn't actually press two opposite directions at once */
					if (right && left) { right = 0; left = 0; }
					if (up && down) { up = 0; down = 0; }

					if (!(mmu->io.joyp & 0x10)) /* directions */
					{
						input |= (right		? 0 : 0b00000001);
						input |= (left		? 0 : 0b00000010);
						input |= (up		? 0 : 0b00000100);
						input |= (down		? 0 : 0b00001000);
					}
					else if (!(mmu->io.joyp & 0x20)) /* actions */
					{
						input |= (a			? 0 : 0b00000001);
						input |= (b			? 0 : 0b00000010);
						input |= (select	? 0 : 0b00000100);
						input |= (start		? 0 : 0b00001000);
					}
					else
					{
						input |= 0b00001111;
					}
					mmu->io.joyp = input | original;
					return &mmu->io.joyp;
				}
				case 0x10: case 0x20: case 0x30:
				case 0x40: case 0x50: case 0x60:
				case 0x70:
					return &mmu->memory.io[address - 0xFF00];
				}
				return &mmu->memory.hram[address - 0xFF80];
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
	case MMAP_IO_BGPD:
		return mmu->palette.background[mmu->io.bgpi & 0x3F];
	case MMAP_IO_OBPD:
		return mmu->palette.foreground[mmu->io.obpi & 0x3F];
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
			uint8_t input = mmu->io.joyp;
			mmu->io.joyp = (value & 0x30) | (input & 0xCF); // only permit writing to bits 4 & 5
			return;
		}
		case 0xFF46: /* dma transfer */
			for (uint16_t copy_addr = value << 8; (copy_addr & 0xFF) < 0x9F; copy_addr++)
			{
				mmu_poke8(mmu, 0xFE00 + (copy_addr & 0xFF), mmu_peek8(mmu, copy_addr));
			}
			return;
		case 0xFF69:
			mmu->palette.background[mmu->io.bgpi & 0x3F] = value;
			if (mmu->io.bgpi & 0x80)
			{
				mmu->io.bgpi = (((mmu->io.bgpi & 0x3F) + 1) & 0x3F) | 0x80;
			}
			return;
		case 0xFF6B:
			mmu->palette.foreground[mmu->io.obpi & 0x3F] = value;
			if (mmu->io.obpi & 0x80)
			{
				mmu->io.obpi = (((mmu->io.obpi & 0x3F) + 1) & 0x3F) | 0x80;
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
			mmu->memory.cart[1] = mmu->memory.cart[0] + (0x4000 * (value ? value : 1)); // todo: check this should go up to 1
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
