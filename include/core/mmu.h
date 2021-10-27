
#ifndef MMU_H
#define MMU_H

#include <stdint.h>
#include "rom.h"

typedef struct
{
	/* rom */
	rom_t* rom;

	/* memory map */
	uint8_t* cart[2];
    uint8_t vram[0x2000];
    uint8_t xram[0x2000];
    uint8_t wram[8][0x1000];
    uint8_t oam[0x9F];
    uint8_t io[0x7F];
    uint8_t hram[0x7E];
    uint8_t ier;
} mmu_t;

void mmu_create(mmu_t* mmu, rom_t* rom);
void mmu_destroy(mmu_t* mmu);

uint8_t* mmu_map(mmu_t* mmu, uint16_t address);

uint8_t mmu_peek8(mmu_t* mmu, uint16_t address);
uint16_t mmu_peek16(mmu_t* mmu, uint16_t address);

void mmu_poke8(mmu_t* mmu, uint16_t address, uint8_t value);
void mmu_poke16(mmu_t* mmu, uint16_t address, uint16_t value);

#endif
