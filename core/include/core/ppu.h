
#ifndef PPU_H
#define PPU_H

#include "mmu.h"
#include "cpu.h"

#define CYCLES_ELAPSED(ppu, num_cycles) (ppu->cycles >= num_cycles)

#define CYCLES_H_BLANK		0xCF
#define CYCLES_OAM_ACCESS	0x53
#define CYCLES_LCD_TRANSFER	0xAF
#define CYCLES_LINE			456

#define SCANLINE_V_BLANK	144
#define SCANLINE_MAX		153

typedef enum
{
	MODE_H_BLANK,
	MODE_OAM,
	MODE_LCD_TRANSFER,
	MODE_V_BLANK
} ppu_mode_t;

typedef struct
{
	ppu_mode_t mode;
	uint32_t cycles;
	uint8_t line;
} ppu_t;

void ppu_create(ppu_t* ppu);
void ppu_destroy(ppu_t* ppu);

void ppu_update_ly(ppu_t* ppu, mmu_t* mmu);
void ppu_cycle(ppu_t* ppu, mmu_t* mmu, cpu_t* cpu, size_t cycles);

#endif
