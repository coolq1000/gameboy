
#ifndef MMU_H
#define MMU_H

#include <stdint.h>
#include "rom.h"

#define GET_BIT(value, index) (((value) & (1 << index)) != 0)
#define SET_BIT(value, index) ((value) | (1 << index))
#define CLEAR_BIT(value, index) ((value) & ~(1 << index))

#define VRAM_SIZE 0x2000
#define XRAM_SIZE 0x2000
#define WRAM_SIZE 0x1000
#define  OAM_SIZE 0x9F
#define   IO_SIZE 0x7F
#define HRAM_SIZE 0x7E

#define MBC5_XRAM_COUNT 0xF
#define CGB_VRAM_COUNT 0x2
#define CGB_WRAM_COUNT 0x8
#define CGB_PALETTE_COUNT 0x40

#define MMAP_ROM_00 0x0000
#define MMAP_ROM_01 0x4000
#define MMAP_VRAM 0x8000
#define MMAP_XRAM 0xA000
#define MMAP_WRAM 0xC000
#define MMAP_OAM 0xFE00
#define MMAP_IO 0xFF00
#define MMAP_IO_DIV 0xFF04
#define MMAP_IO_TIMA 0xFF05
#define MMAP_IO_TMA 0xFF06
#define MMAP_IO_TAC 0xFF07
#define MMAP_IO_IRF 0xFF0F
#define MMAP_IO_LCDC 0xFF40
#define MMAP_IO_STAT 0xFF41
#define MMAP_IO_SCY 0xFF42
#define MMAP_IO_SCX 0xFF43
#define MMAP_IO_LY 0xFF44
#define MMAP_IO_LYC 0xFF45
#define MMAP_IO_BGP 0xFF47
#define MMAP_IO_OBP0 0xFF48
#define MMAP_IO_OBP1 0xFF49
#define MMAP_IO_WY 0xFF4A
#define MMAP_IO_WX 0xFF4B
#define MMAP_IO_VBK 0xFF4F
#define MMAP_IO_BGPI 0xFF68
#define MMAP_IO_BGPD 0xFF69
#define MMAP_IO_OBPI 0xFF6A
#define MMAP_IO_OBPD 0xFF6B
#define MMAP_IO_SVBK 0xFF70
#define MMAP_HRAM 0xFF80
#define MMAP_IE 0xFFFF

extern int flag_hit;

typedef struct
{
	/* rom */
	rom_t* rom;

	/* memory map */
    struct
    {
        uint8_t* cart[2];
        uint8_t* vram[CGB_VRAM_COUNT];
        uint8_t* xram[MBC5_XRAM_COUNT];
        uint8_t* wram[CGB_WRAM_COUNT];
        uint8_t* oam;
        uint8_t* io;
        uint8_t* hram;
        uint8_t interrupt_enable;
    } memory;

    struct
    {
        uint8_t joyp;
        uint8_t div;
        uint8_t tima;
        uint8_t tma;
        uint8_t tac;
        uint8_t irf;
        uint8_t lcdc;
        uint8_t stat;
        uint8_t scy;
        uint8_t scx;
        uint8_t ly;
        uint8_t lyc;
        uint8_t bgp;
        uint8_t obp0;
        uint8_t obp1;
        uint8_t wy;
        uint8_t wx;
        uint8_t vbk; // only last bit readable
        uint8_t bgpi;
        uint8_t bgpd;
        uint8_t obpi;
        uint8_t obpd;
        uint8_t svbk; // only last two bits readable
    } io;

    struct
    {

    } mbc;

    struct
    {
        uint8_t background[CGB_PALETTE_COUNT];
        uint8_t foreground[CGB_PALETTE_COUNT];
    } palette;

    struct
    {
        uint8_t start, select;
        uint8_t a, b;
        uint8_t down, up, left, right;
    } buttons;

    uint8_t null_mem;
} mmu_t;

void mmu_create(mmu_t* mmu, rom_t* rom);
void mmu_destroy(mmu_t* mmu);

uint8_t* mmu_map(mmu_t* mmu, uint16_t address);

uint8_t mmu_peek8(mmu_t* mmu, uint16_t address);
uint16_t mmu_peek16(mmu_t* mmu, uint16_t address);

void mmu_poke8(mmu_t* mmu, uint16_t address, uint8_t value);
void mmu_poke16(mmu_t* mmu, uint16_t address, uint16_t value);

#endif
