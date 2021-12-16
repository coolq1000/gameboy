
#ifndef PPU_H
#define PPU_H

#include "mmu.h"
#include "cpu.h"
#include "util.h"

#define CYCLES_ELAPSED(ppu, num_cycles) (ppu->cycles >= num_cycles)

#define CYCLES_H_BLANK		207
#define CYCLES_OAM_ACCESS	83
#define CYCLES_LCD_TRANSFER	175
#define CYCLES_LINE			456

#define SCANLINE_V_BLANK	144
#define SCANLINE_MAX		153

#define LCD_WIDTH 160
#define LCD_HEIGHT 144

#define MAX_SPRITES 10

typedef enum ppu_mode_t
{
	MODE_H_BLANK,
	MODE_V_BLANK,
	MODE_OAM,
	MODE_LCD_TRANSFER
} ppu_mode_t;

typedef struct ppu_t
{
	ppu_mode_t mode;
	u32 cycles;
	u8 line;
	u32 lcd[LCD_WIDTH * LCD_HEIGHT];
	void (*v_blank_callback)();
	bool is_cgb;
} ppu_t;

extern u8 ppu_palette[12];

void ppu_create(ppu_t* ppu, bool is_cgb);
void ppu_destroy(ppu_t* ppu);

void ppu_update_ly(ppu_t* ppu, mmu_t* mmu);
void ppu_cycle(ppu_t* ppu, mmu_t* mmu, cpu_t* cpu, usize cycles);

void ppu_set_pixel(ppu_t* ppu, usize x, usize y, u32 value);
u32 ppu_get_pixel(ppu_t* ppu, usize x, usize y);

u8 ppu_get_tile(ppu_t* ppu, mmu_t* mmu, u8 tile_id, usize tile_x, usize tile_y, bool is_sprite, u8 vram_bank);
u8 ppu_convert_dmg_palette(u8 palette, u8 color_id);
u16 ppu_convert_cgb_palette(mmu_t* mmu, u8* palette, u8 palette_id, u8 color_id);

u32 ppu_render_background(ppu_t* ppu, mmu_t* mmu, u8 x, u8 y, u8 is_window);
void ppu_render_sprites(ppu_t* ppu, mmu_t* mmu, usize x, usize y);
void ppu_render_line(ppu_t* ppu, mmu_t* mmu, usize line);

#endif
