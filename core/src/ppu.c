#include "core/ppu.h"

#include <stdio.h>

uint8_t ppu_palette[12] =
{
	0xE0, 0xF8, 0xD0,
	0x88, 0xC0, 0x70,
	0x34, 0x68, 0x56,
	0x08, 0x18, 0x20
};

// uint8_t ppu_palette[4][3] =
// {
// 	{ 0xFF, 0xFF, 0xFF },
// 	{ 0xC0, 0xC0, 0xC0 },
// 	{ 0x50, 0x50, 0x50 },
// 	{ 0x00, 0x00, 0x00 }
// };

void ppu_create(ppu_t* ppu, bool is_cgb)
{
	ppu->mode = MODE_OAM;
	ppu->cycles = 0;
	ppu->line = 0; // todo: check this
	ppu->v_blank_callback = NULL;
	ppu->is_cgb = is_cgb;

	for (size_t i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++)
		ppu->lcd[i] = 0;
}

void ppu_destroy(ppu_t* ppu)
{

}

void ppu_update_ly(ppu_t* ppu, mmu_t* mmu)
{
	ppu->line = (ppu->line + 1) % SCANLINE_MAX;
	mmu_poke8(mmu, MMAP_IO_LY, ppu->line);
}

void ppu_compare_ly_lyc(ppu_t* ppu, mmu_t* mmu, cpu_t* cpu)
{
	if (mmu_peek8(mmu, MMAP_IO_LY) == mmu_peek8(mmu, MMAP_IO_LYC))
	{
		mmu_poke8(mmu, MMAP_IO_STAT, mmu_peek8(mmu, MMAP_IO_STAT) | 0x4);

		if (cpu->interrupt.master)
		{
			cpu_request(cpu, mmu, INT_LCD_STAT_INDEX);
		}
	}
	else
	{
		mmu_poke8(mmu, MMAP_IO_STAT, mmu_peek8(mmu, MMAP_IO_STAT) & ~0x4);
	}
}

void ppu_set_stat_mode(ppu_t* ppu, mmu_t* mmu, cpu_t* cpu)
{
	uint8_t mask = (1 << (3 + ppu->mode));

	if (cpu->interrupt.master)
	{
		if (mmu_peek8(mmu, MMAP_IO_STAT) & mask)
		{
			cpu_request(cpu, mmu, INT_LCD_STAT_INDEX);
		}
	}
}

void ppu_cycle(ppu_t* ppu, mmu_t* mmu, cpu_t* cpu, size_t cycles)
{
	ppu->cycles += cycles;

	switch (ppu->mode)
	{
	case MODE_H_BLANK:
		if (CYCLES_ELAPSED(ppu, CYCLES_H_BLANK))
		{
			ppu_update_ly(ppu, mmu);
			ppu_compare_ly_lyc(ppu, mmu, cpu);

			if (ppu->line == SCANLINE_V_BLANK)
			{
				cpu_request(cpu, mmu, INT_V_BLANK_INDEX);
				if (ppu->v_blank_callback) ppu->v_blank_callback();
				ppu->mode = MODE_V_BLANK;
			}
			else
			{
				ppu->mode = MODE_OAM;
			}

			ppu_set_stat_mode(ppu, mmu, cpu);
			ppu->cycles -= CYCLES_H_BLANK;
		}
		break;
	case MODE_OAM:
		if (CYCLES_ELAPSED(ppu, CYCLES_OAM_ACCESS))
		{
			ppu->mode = MODE_LCD_TRANSFER;
			ppu->cycles -= CYCLES_OAM_ACCESS;
		}
		break;
	case MODE_LCD_TRANSFER:
		if (CYCLES_ELAPSED(ppu, CYCLES_LCD_TRANSFER))
		{
			ppu_render_line(ppu, mmu, ppu->line);
			ppu->mode = MODE_H_BLANK;
			// todo: hdma transfer
			ppu_set_stat_mode(ppu, mmu, cpu);
			ppu->cycles -= CYCLES_LCD_TRANSFER;
		}
		break;
	case MODE_V_BLANK:
		if (CYCLES_ELAPSED(ppu, CYCLES_LINE))
		{
			ppu_update_ly(ppu, mmu);
			ppu_compare_ly_lyc(ppu, mmu, cpu);

			if (ppu->line == 0)
			{
				ppu->mode = MODE_OAM;
				ppu_set_stat_mode(ppu, mmu, cpu);
			}

			ppu->cycles -= CYCLES_LINE;
		}
		break;
	}

	mmu_poke8(mmu, MMAP_IO_STAT, (mmu_peek8(mmu, MMAP_IO_STAT) & 0xFC) | ppu->mode);
}

void ppu_set_pixel(ppu_t* ppu, size_t x, size_t y, uint32_t value)
{
	ppu->lcd[x + y * LCD_WIDTH] = value;
}

uint32_t ppu_get_pixel(ppu_t* ppu, size_t x, size_t y)
{
	return ppu->lcd[x + y * LCD_WIDTH];
}

uint8_t ppu_get_tile(ppu_t* ppu, mmu_t* mmu, uint8_t tile_id, size_t tile_x, size_t tile_y, bool is_sprite, uint8_t vram_bank)
{
	uint16_t offset = 0x00;
	uint8_t addressing_8800 = !(mmu_peek8(mmu, MMAP_IO_LCDC) & 0x10);

	if (!is_sprite && addressing_8800)
	{
		if (tile_id <= 0x7F) { offset = 0x1000; }
		else { offset = 0x800; tile_id -= 0x80; }
	}

	uint16_t tile_addr = tile_id * 8 * 2;

	/* read out each line part, assumes little-endian */
	uint8_t pixel_line_1, pixel_line_2;

	if (ppu->is_cgb)
	{
		pixel_line_1 = mmu->memory.vram[vram_bank][(tile_addr + tile_y * 2) + 1 + offset];
		pixel_line_2 = mmu->memory.vram[vram_bank][(tile_addr + tile_y * 2) + 0 + offset];
	}
	else
	{
		pixel_line_1 = mmu_peek8(mmu, MMAP_VRAM + (tile_addr + tile_y * 2) + 1 + offset);
		pixel_line_2 = mmu_peek8(mmu, MMAP_VRAM + (tile_addr + tile_y * 2) + 0 + offset);
	}

	uint8_t pixel_mask = 0x80 >> tile_x;
	uint8_t pixel = (((pixel_line_1 & pixel_mask) != 0) << 1) | ((pixel_line_2 & pixel_mask) != 0);

	return pixel;
}

uint8_t ppu_convert_dmg_palette(uint8_t palette, uint8_t color_id)
{
	return ((palette >> (color_id * 2)) & 0x3);
}

uint16_t ppu_convert_cgb_palette(mmu_t* mmu, uint8_t* palette, uint8_t palette_id, uint8_t color_id)
{
	uint8_t id = palette_id * 8 + color_id * 2;
	return palette[id] | (palette[id + 1] << 8);
}

uint32_t ppu_apply_dmg_palette(uint8_t* palette, uint16_t palette_id)
{
	uint8_t r = palette[palette_id * 3 + 0];
	uint8_t g = palette[palette_id * 3 + 1];
	uint8_t b = palette[palette_id * 3 + 2];

	return (b << 16) | (g << 8) | r;
}

uint32_t ppu_apply_cgb_palette(uint16_t raw_color)
{
	uint32_t r = ((raw_color >> 0) & 0x1F) << 3;
	uint32_t g = ((raw_color >> 5) & 0x1F) << 3;
	uint32_t b = ((raw_color >> 10) & 0x1F) << 3;

	return (0xFF << 24) | (b << 16) | (g << 8) | r;
}

uint32_t ppu_render_background(ppu_t* ppu, mmu_t* mmu, uint8_t x, uint8_t y, uint8_t is_window)
{
	uint16_t map_area;

	if (is_window) map_area = 0x9C00;
	else map_area = (mmu_peek8(mmu, MMAP_IO_LCDC) & 0x8) ? 0x9C00 : 0x9800;

	uint8_t tile_x = x / 8;
	uint8_t tile_y = y / 8;

	uint8_t cgb_attributes = mmu->memory.vram[1][0x1800 + (tile_x + tile_y * 32)];

	uint8_t tile_pixel_x = x % 8;
	uint8_t tile_pixel_y = y % 8;

	if (cgb_attributes & 0x20 && !is_window) tile_pixel_x = 7 - tile_pixel_x;
	if (cgb_attributes & 0x40 && !is_window) tile_pixel_y = 7 - tile_pixel_y;

	uint8_t cgb_vbank = (cgb_attributes & 0x8) ? 1 : 0;

	uint8_t tile_id = mmu->memory.vram[cgb_vbank][(map_area + tile_x + tile_y * 32) - MMAP_VRAM];
	uint8_t tile = ppu_get_tile(ppu, mmu, tile_id, tile_pixel_x, tile_pixel_y, false, cgb_vbank);
	
	if (!ppu->is_cgb)
	{
		return ppu_apply_dmg_palette(ppu_palette, ppu_convert_dmg_palette(mmu->io.bgp, tile));
	}
	else
	{
		uint8_t cgb_palette = cgb_attributes & 0x7;
		return ppu_apply_cgb_palette(ppu_convert_cgb_palette(mmu, mmu->palette.background, !is_window ? cgb_palette : 1, tile));
	}
}

void ppu_render_sprites(ppu_t* ppu, mmu_t* mmu, size_t x, size_t y)
{
	uint8_t sprite_height = (mmu_peek8(mmu, MMAP_IO_LCDC) & 0x4) ? 16 : 8;

	size_t max_draw = 10;
	for (size_t i_sprite = 40; i_sprite-- > 0 && max_draw;) // iterate backwards to ensure sprite ordering
	{
		uint16_t sprite_address = i_sprite * 4;

		int16_t sprite_x = mmu->memory.oam[sprite_address + 1] - 8;
		int16_t sprite_y = mmu->memory.oam[sprite_address + 0] - 16;
		uint8_t sprite_tile_id = mmu->memory.oam[sprite_address + 2] & (sprite_height == 8 ? 0xFF : 0xFE);
		uint8_t sprite_tile_attributes = mmu->memory.oam[sprite_address + 3];

		uint8_t palette_0 = mmu->io.obp0;
		uint8_t palette_1 = mmu->io.obp1;

		uint8_t palette = sprite_tile_attributes & 0x10 ? palette_1 : palette_0;

		/* check sprite active */
		if (mmu->memory.oam[sprite_address]) // checks y coordinate != 0
		{
			/* check sprite bounds */
			if ((int16_t)y >= sprite_y && (int16_t)y < sprite_y + sprite_height)
			{
				if ((int16_t)x >= sprite_x && (int16_t)x < sprite_x + 8)
				{
					uint8_t cgb_palette = sprite_tile_attributes & 0x7;
					uint8_t flip_x = sprite_tile_attributes & 0x20;
					uint8_t flip_y = sprite_tile_attributes & 0x40;
					uint8_t priority = !(sprite_tile_attributes & 0x80);

					int16_t pixel_x = (x - sprite_x) % 8;
					int16_t pixel_y = (y - sprite_y) % sprite_height;

					if (flip_x) pixel_x = 7 - pixel_x;
					if (flip_y) pixel_y = sprite_height - 1 - pixel_y;

					uint8_t pixel = ppu_get_tile(ppu, mmu, sprite_tile_id, pixel_x, pixel_y, true, (sprite_tile_attributes & 0x8) ? 1 : 0);

					uint32_t palette_pixel;
					if (!ppu->is_cgb) palette_pixel = ppu_apply_dmg_palette(ppu_palette, ppu_convert_dmg_palette(palette, pixel));
					else palette_pixel = ppu_apply_cgb_palette(ppu_convert_cgb_palette(mmu, mmu->palette.foreground, cgb_palette, pixel));

					if (pixel) ppu_set_pixel(ppu, x, y, palette_pixel);
				}

				max_draw--;
			}
		}
	}
}

void ppu_render_line(ppu_t* ppu, mmu_t* mmu, size_t line)
{
	uint8_t scroll_x = mmu_peek8(mmu, MMAP_IO_SCX);
	uint8_t scroll_y = mmu_peek8(mmu, MMAP_IO_SCY);

	uint8_t lcd_enable = (mmu_peek8(mmu, MMAP_IO_LCDC) & 0x80) != 0;
	uint8_t background_enable = (mmu_peek8(mmu, MMAP_IO_LCDC) & 0x1) != 0;
	uint8_t sprites_enable = (mmu_peek8(mmu, MMAP_IO_LCDC) & 0x2) != 0;
	uint8_t window_enable = (mmu_peek8(mmu, MMAP_IO_LCDC) & 0x20) != 0;

	int window_x = ((int)mmu_peek8(mmu, MMAP_IO_WX)) - 7;
	int window_y = mmu_peek8(mmu, MMAP_IO_WY);

	if (lcd_enable)
	{
		for (size_t x = 0; x < LCD_WIDTH; x++)
		{
			if (background_enable) // render background
			{
				ppu_set_pixel(ppu, x, line, ppu_render_background(ppu, mmu, x + scroll_x, line + scroll_y, false));
			}
			if (window_enable)
			{
				if (line >= window_y && (int)x >= window_x)
				{
					ppu_set_pixel(ppu, x, line, ppu_render_background(ppu, mmu, x - window_x, line - window_y, true)); // todo check LCDC.3 only if not a window
				}
			}
			if (sprites_enable) // render sprites
			{
				ppu_render_sprites(ppu, mmu, x, line);
			}
		}
	}
	// else
	// {
	// 	for (size_t x = 0; x < LCD_WIDTH; x++)
	// 	{
	// 		ppu_set_pixel(ppu, x, line, 0);
	// 	}
	// }
}
