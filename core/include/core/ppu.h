
#ifndef PPU_H
#define PPU_H

#include "mmu.h"
#include "cpu.h"

#define CYCLES_ELAPSED(ppu, num_cycles) (ppu->cycles >= num_cycles)

#define CYCLES_H_BLANK		207
#define CYCLES_OAM_ACCESS	83
#define CYCLES_LCD_TRANSFER	175
#define CYCLES_LINE			456

#define SCANLINE_V_BLANK	144
#define SCANLINE_MAX		153

#define LCD_WIDTH 160
#define LCD_HEIGHT 144

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
	uint8_t lcd[LCD_WIDTH * LCD_HEIGHT];
	void (*v_blank_callback)();
} ppu_t;

extern uint8_t ppu_palette[4][3];

void ppu_create(ppu_t* ppu);
void ppu_destroy(ppu_t* ppu);

void ppu_update_ly(ppu_t* ppu, mmu_t* mmu);
void ppu_cycle(ppu_t* ppu, mmu_t* mmu, cpu_t* cpu, size_t cycles);

void ppu_set_pixel(ppu_t* ppu, size_t x, size_t y, uint8_t value);
uint8_t ppu_get_pixel(ppu_t* ppu, size_t x, size_t y);

uint8_t ppu_get_tile(ppu_t* ppu, mmu_t* mmu, uint8_t tile_id, size_t tile_x, size_t tile_y, bool is_sprite);
uint8_t ppu_convert_palette(ppu_t* ppu, uint8_t pixel, uint8_t palette);

uint8_t ppu_render_background(ppu_t* ppu, mmu_t* mmu, size_t x, size_t y, uint8_t is_window);
void ppu_render_sprites(ppu_t* ppu, mmu_t* mmu, size_t x, size_t y, uint8_t background);
void ppu_render_line(ppu_t* ppu, mmu_t* mmu, size_t line);

#endif
