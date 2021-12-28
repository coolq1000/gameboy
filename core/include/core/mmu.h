
#ifndef MMU_H
#define MMU_H

#include "rom.h"
#include "util.h"

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
#define MMAP_IO_JOYP 0xFF00
#define MMAP_IO_DIV 0xFF04
#define MMAP_IO_TIMA 0xFF05
#define MMAP_IO_TMA 0xFF06
#define MMAP_IO_TAC 0xFF07
#define MMAP_IO_IRF 0xFF0F
#define MMAP_IO_NR52 0xFF26
#define MMAP_IO_LCDC 0xFF40
#define MMAP_IO_STAT 0xFF41
#define MMAP_IO_SCY 0xFF42
#define MMAP_IO_SCX 0xFF43
#define MMAP_IO_LY 0xFF44
#define MMAP_IO_LYC 0xFF45
#define MMAP_IO_DMA 0xFF46
#define MMAP_IO_BGP 0xFF47
#define MMAP_IO_OBP0 0xFF48
#define MMAP_IO_OBP1 0xFF49
#define MMAP_IO_WY 0xFF4A
#define MMAP_IO_WX 0xFF4B
#define MMAP_IO_KEY1 0xFF4D
#define MMAP_IO_VBK 0xFF4F
#define MMAP_IO_HDMA1 0xFF51 /* src - hi */
#define MMAP_IO_HDMA2 0xFF52 /* src - lo */
#define MMAP_IO_HDMA3 0xFF53 /* dst - hi */
#define MMAP_IO_HDMA4 0xFF54 /* dst - lo */
#define MMAP_IO_HDMA5 0xFF55 /* length/mode/start */
#define MMAP_IO_BGPI 0xFF68
#define MMAP_IO_BGPD 0xFF69
#define MMAP_IO_OBPI 0xFF6A
#define MMAP_IO_OBPD 0xFF6B
#define MMAP_IO_SVBK 0xFF70
#define MMAP_HRAM 0xFF80
#define MMAP_IE 0xFFFF

typedef struct mmu
{
	rom_t* rom;

	/* memory map */
    struct
    {
        u8* cart[2];
        u8* vram[CGB_VRAM_COUNT];
        u8* xram[MBC5_XRAM_COUNT];
        u8* wram[CGB_WRAM_COUNT];
        u8* oam;
        u8* io;
        u8* hram;
        u8 interrupt_enable;
    } memory;
    struct
    {
        u8 joyp;
        u8 div;
        u8 tima;
        u8 tma;
        u8 tac;
        u8 irf;
        u8 lcdc;
        u8 stat;
        u8 scy;
        u8 scx;
        u8 ly;
        u8 lyc;
        u8 dma;
        u8 bgp;
        u8 obp0;
        u8 obp1;
        u8 wy;
        u8 wx;
        union { u8 key1; struct { u8 prepare_speed_switch : 1; u8 _pad_00 : 6; u8 current_speed : 1; }; };
        union { u8 vbk; struct { u8 vram_bank : 1; u8 _pad_01 : 7; }; }; // only first bit readable
        u8 hdma1;
        u8 hdma2;
        u8 hdma3;
        u8 hdma4;
        u8 hdma5;
        u8 bgpi;
        u8 bgpd;
        u8 obpi;
        u8 obpd;
        u8 svbk; // only last two bits readable
    } io;
    // struct
    // {

    // } mbc;
    struct
    {
        u8 background[CGB_PALETTE_COUNT];
        u8 foreground[CGB_PALETTE_COUNT];
    } palette;
    struct
    {
        u8 start, select;
        u8 a, b;
        u8 down, up, left, right;
    } buttons;
    struct
    {
        u16 source;
        u16 destination;
        u16 length;
        u16 to_copy;
        bool hblank;
    } hdma;

    u8 null_mem;
} mmu_t;

void mmu_init(mmu_t* mmu, rom_t* rom);
void mmu_free(mmu_t* mmu);

u8* mmu_map(mmu_t* mmu, u16 address);

u8 mmu_peek(mmu_t* mmu, u16 address);
void mmu_poke(mmu_t* mmu, u16 address, u8 value);

void mmu_hdma_copy_block(mmu_t* mmu);

#endif
