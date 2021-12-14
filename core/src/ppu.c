#include "core/ppu.h"

#include <stdio.h>

u8 ppu_palette[12] =
{
	0xE0, 0xF8, 0xD0,
	0x88, 0xC0, 0x70,
	0x34, 0x68, 0x56,
	0x08, 0x18, 0x20
};

void ppu_create(ppu_t* ppu, bool is_cgb)
{
	ppu->mode = MODE_OAM;
	ppu->cycles = 0;
	ppu->line = 0; // todo: check this
	ppu->v_blank_callback = NULL;
	ppu->is_cgb = is_cgb;

	for (usize i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++)
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
	u8 mask = (1 << (3 + ppu->mode));

	if (cpu->interrupt.master)
	{
		if (mmu_peek8(mmu, MMAP_IO_STAT) & mask)
		{
			cpu_request(cpu, mmu, INT_LCD_STAT_INDEX);
		}
	}
}

void ppu_cycle(ppu_t* ppu, mmu_t* mmu, cpu_t* cpu, usize cycles)
{
	ppu->cycles += cycles / 4;

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
			
			/* hdma transfer */
			if (mmu->hdma.hblank && mmu->hdma.length > 0) mmu->hdma.to_copy = 0x10;

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

void ppu_set_pixel(ppu_t* ppu, usize x, usize y, u32 value)
{
	ppu->lcd[x + y * LCD_WIDTH] = value;
}

u32 ppu_get_pixel(ppu_t* ppu, usize x, usize y)
{
	return ppu->lcd[x + y * LCD_WIDTH];
}

u8 ppu_get_tile(ppu_t* ppu, mmu_t* mmu, u8 tile_id, usize tile_x, usize tile_y, bool is_sprite, u8 vram_bank)
{
	u16 tile_addr;

	if (!(mmu->io.lcdc & 0x10) && !is_sprite) tile_addr = (u16)(0x1000 + 16 * (i8)tile_id);
	else tile_addr = tile_id * 16;

	/* read out each line part, assumes little-endian */
	u8 pixel_line_1, pixel_line_2;

	if (ppu->is_cgb)
	{
		pixel_line_1 = mmu->memory.vram[vram_bank][(tile_addr + tile_y * 2) + 1];
		pixel_line_2 = mmu->memory.vram[vram_bank][(tile_addr + tile_y * 2) + 0];
	}
	else
	{
		pixel_line_1 = mmu_peek8(mmu, MMAP_VRAM + (tile_addr + tile_y * 2) + 1);
		pixel_line_2 = mmu_peek8(mmu, MMAP_VRAM + (tile_addr + tile_y * 2) + 0);
	}

	u8 pixel_mask = 0x80 >> tile_x;
	u8 pixel = (((pixel_line_1 & pixel_mask) != 0) << 1) | ((pixel_line_2 & pixel_mask) != 0);

	return pixel;
}

u8 ppu_convert_dmg_palette(u8 palette, u8 color_id)
{
	return ((palette >> (color_id * 2)) & 0x3);
}

u16 ppu_convert_cgb_palette(mmu_t* mmu, u8* palette, u8 palette_id, u8 color_id)
{
	u8 id = palette_id * 8 + color_id * 2;
	return palette[id] | (palette[id + 1] << 8);
}

u32 ppu_apply_dmg_palette(u8* palette, u16 palette_id)
{
	u8 r = palette[palette_id * 3 + 0];
	u8 g = palette[palette_id * 3 + 1];
	u8 b = palette[palette_id * 3 + 2];

	return (b << 16) | (g << 8) | r;
}

u32 ppu_apply_cgb_palette(u16 raw_color)
{
	u32 r = ((raw_color >> 0) & 0x1F) << 3;
	u32 g = ((raw_color >> 5) & 0x1F) << 3;
	u32 b = ((raw_color >> 10) & 0x1F) << 3;

	return (0xFF << 24) | (b << 16) | (g << 8) | r;
}

u32 ppu_render_background(ppu_t* ppu, mmu_t* mmu, u8 x, u8 y, u8 is_window)
{
	u16 map_area = (mmu_peek8(mmu, MMAP_IO_LCDC) & 0x8) || is_window ? 0x1C00 : 0x1800;

	u8 tile_x = x / 8;
	u8 tile_y = y / 8;

	u8 cgb_attributes = mmu->memory.vram[1][0x1800 + (tile_x + tile_y * 32)];

	u8 tile_pixel_x = x % 8;
	u8 tile_pixel_y = y % 8;

	if (cgb_attributes & 0x20) tile_pixel_x = 7 - tile_pixel_x;
	if (cgb_attributes & 0x40) tile_pixel_y = 7 - tile_pixel_y;

	u8 cgb_vbank = (cgb_attributes & 0x8) ? 1 : 0;

	u8 tile_id = mmu->memory.vram[0][map_area + tile_x + tile_y * 32];
	u8 tile = ppu_get_tile(ppu, mmu, tile_id, tile_pixel_x, tile_pixel_y, false, cgb_vbank);
	
	if (!ppu->is_cgb)
	{
		return ppu_apply_dmg_palette(ppu_palette, ppu_convert_dmg_palette(mmu->io.bgp, tile));
	}
	else
	{
		u8 cgb_palette = cgb_attributes & 0x7;
		return ppu_apply_cgb_palette(ppu_convert_cgb_palette(mmu, mmu->palette.background, cgb_palette, tile));
	}
}

void ppu_render_sprites(ppu_t* ppu, mmu_t* mmu, usize x, usize y)
{
	u8 sprite_height = (mmu_peek8(mmu, MMAP_IO_LCDC) & 0x4) ? 16 : 8;

	usize max_draw = 10;
	for (usize i_sprite = 40; i_sprite-- > 0 && max_draw;) // iterate backwards to ensure sprite ordering
	{
		u16 sprite_address = i_sprite * 4;

		i16 sprite_x = mmu->memory.oam[sprite_address + 1] - 8;
		i16 sprite_y = mmu->memory.oam[sprite_address + 0] - 16;
		u8 sprite_tile_id = mmu->memory.oam[sprite_address + 2] & (sprite_height == 8 ? 0xFF : 0xFE);
		u8 sprite_tile_attributes = mmu->memory.oam[sprite_address + 3];

		u8 palette = sprite_tile_attributes & 0x10 ? mmu->io.obp1 : mmu->io.obp0;

		/* check sprite active */
		if (mmu->memory.oam[sprite_address]) // checks y coordinate != 0
		{
			/* check sprite bounds */
			if ((i16)y >= sprite_y && (i16)y < sprite_y + sprite_height)
			{
				if ((i16)x >= sprite_x && (i16)x < sprite_x + 8)
				{
					u8 cgb_palette = sprite_tile_attributes & 0x7;
					u8 flip_x = sprite_tile_attributes & 0x20;
					u8 flip_y = sprite_tile_attributes & 0x40;
					u8 priority = !(sprite_tile_attributes & 0x80);

					i16 pixel_x = (x - sprite_x) % 8;
					i16 pixel_y = (y - sprite_y) % sprite_height;

					if (flip_x) pixel_x = 7 - pixel_x;
					if (flip_y) pixel_y = sprite_height - 1 - pixel_y;

					u8 pixel = ppu_get_tile(ppu, mmu, sprite_tile_id, pixel_x, pixel_y, true, (sprite_tile_attributes & 0x8) ? 1 : 0);

					u32 palette_pixel;
					if (!ppu->is_cgb) palette_pixel = ppu_apply_dmg_palette(ppu_palette, ppu_convert_dmg_palette(palette, pixel));
					else palette_pixel = ppu_apply_cgb_palette(ppu_convert_cgb_palette(mmu, mmu->palette.foreground, cgb_palette, pixel));

					if (pixel) ppu_set_pixel(ppu, x, y, palette_pixel);
				}

				// max_draw--; // todo: figure out maximum draw
			}
		}
	}
}

void ppu_debug_bg(ppu_t* ppu, mmu_t* mmu, usize x, usize y)
{
	u8 tile_x = (x / 8);
	u8 tile_id = tile_x + ((y / 8) * 16);

	if (tile_x <= 0xF)
	{
		u8 pixel = 255 - (ppu_get_tile(ppu, mmu, tile_id, x % 8, y % 8, false, 1) * 85);
		ppu_set_pixel(ppu, x, y, 0xFF000000 | (pixel << 16) | (pixel << 8) | (pixel));
	}
}

void ppu_render_line(ppu_t* ppu, mmu_t* mmu, usize line)
{
	u8 scroll_x = mmu->io.scx;
	u8 scroll_y = mmu->io.scy;

	u8 lcd_enable = (mmu->io.lcdc & 0x80) != 0;
	u8 background_enable = (mmu->io.lcdc & 0x1) != 0;
	u8 sprites_enable = (mmu->io.lcdc & 0x2) != 0;
	u8 window_enable = (mmu->io.lcdc & 0x20) != 0;

	int window_x = ((int)mmu->io.wx) - 7;
	int window_y = mmu->io.wy;

	if (lcd_enable)
	{
		for (usize x = 0; x < LCD_WIDTH; x++)
		{
			if (window_enable && (line >= window_y && (int)x >= window_x))
			{
				ppu_set_pixel(ppu, x, line, ppu_render_background(ppu, mmu, x - window_x, line - window_y, true)); // todo check LCDC.3 only if not a window
			}
			else if (background_enable) // render background
			{
				ppu_set_pixel(ppu, x, line, ppu_render_background(ppu, mmu, x + scroll_x, line + scroll_y, false));
			}

			if (sprites_enable) // render sprites
			{
				ppu_render_sprites(ppu, mmu, x, line);
			}

			// ppu_debug_bg(ppu, mmu, x, line);
		}
	}
	// else
	// {
	// 	for (usize x = 0; x < LCD_WIDTH; x++)
	// 	{
	// 		ppu_set_pixel(ppu, x, line, 0);
	// 	}
	// }
}
