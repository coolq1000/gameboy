#include "core/ppu.h"

#include <stdio.h>

uint8_t ppu_palette[4][3] =
{
	{ 0xE0, 0xF8, 0xD0 },
	{ 0x88, 0xC0, 0x70 },
	{ 0x34, 0x68, 0x56 },
	{ 0x08, 0x18, 0x20 }
};

// uint8_t ppu_palette[4][3] =
// {
// 	{ 0xFF, 0xFF, 0xFF },
// 	{ 0xC0, 0xC0, 0xC0 },
// 	{ 0x50, 0x50, 0x50 },
// 	{ 0x00, 0x00, 0x00 }
// };

void ppu_create(ppu_t* ppu)
{
	ppu->mode = MODE_OAM;
	ppu->cycles = 0;
	ppu->line = 0; // todo: check this
	ppu->v_blank_callback = NULL;

	for (size_t i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++)
		ppu->lcd[i] = 0;
}

void ppu_destroy(ppu_t* ppu)
{

}

void ppu_update_ly(ppu_t* ppu, mmu_t* mmu)
{
	ppu->line = (ppu->line + 1) % SCANLINE_MAX;
	mmu->io[MMAP_IO_LY & 0xFF] = ppu->line;
	// todo: check lyc in here?
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
			// todo: check ly == lyc

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

			// todo: set h-blank stat bit
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
			// todo: check lcd-transfer stat bit
			ppu->cycles -= CYCLES_LCD_TRANSFER;
		}
		break;
	case MODE_V_BLANK:
		if (CYCLES_ELAPSED(ppu, CYCLES_LINE))
		{
			ppu_update_ly(ppu, mmu);
			// todo: check ly == lyc

			if (ppu->line == 0)
			{
				ppu->mode = MODE_OAM;
				// todo: check lcd-transfer stat bit
			}

			ppu->cycles -= CYCLES_LINE;
		}
		break;
	}
}

void ppu_set_pixel(ppu_t* ppu, size_t x, size_t y, uint8_t value)
{
	ppu->lcd[x + y * LCD_WIDTH] = value;
}

uint8_t ppu_get_pixel(ppu_t* ppu, size_t x, size_t y)
{
	return ppu->lcd[x + y * LCD_WIDTH];
}

uint8_t ppu_get_tile(ppu_t* ppu, mmu_t* mmu, uint8_t tile_id, size_t tile_x, size_t tile_y, uint8_t palette, bool is_sprite)
{
	uint16_t offset = 0x00;
	uint8_t addressing_8800 = !(mmu->io[MMAP_IO_LCDC - 0xFF00] & 0x10);

	if (!is_sprite && addressing_8800)
	{
		if (tile_id <= 0x7F) { offset = 0x1000; }
		else { offset = 0x800; tile_id -= 0x80; }
	}

	uint16_t tile_addr = tile_id * 8 * 2;

	uint8_t pixel_x = tile_x % 8;
	uint8_t pixel_y = tile_y % 8;

	/* read out each line part, assumes little-endian */
	uint8_t pixel_line_1 = mmu->vram[(tile_addr + pixel_y * 2) + 1 + offset];
	uint8_t pixel_line_2 = mmu->vram[(tile_addr + pixel_y * 2) + 0 + offset];

	uint8_t pixel_mask = 0x80 >> pixel_x;
	uint8_t pixel = (((pixel_line_1 & pixel_mask) != 0) << 1) | ((pixel_line_2 & pixel_mask) != 0);

	uint8_t palette_shift = pixel * 2;
	if (is_sprite && pixel == 0) return 0xBC;
	return ((palette & (0x03 << palette_shift)) >> palette_shift) - 1;
}

void ppu_render_background(ppu_t* ppu, mmu_t* mmu, size_t line)
{
	uint8_t window_x = mmu->io[0x43];
	uint8_t window_y = mmu->io[0x42];

	for (size_t x = 0; x < LCD_WIDTH; x++)
	{
		size_t pixel_x = x + window_x;
		size_t pixel_y = line + window_y;

		size_t tile_x = pixel_x / 8;
		size_t tile_y = pixel_y / 8;

		uint8_t tile_id = mmu_peek8(mmu, 0x9800 + tile_x + tile_y * 32);

		uint8_t tile_pixel = ppu_get_tile(ppu, mmu, tile_id, pixel_x % 256, pixel_y % 256, mmu->io[0x47], false);
		ppu_set_pixel(ppu, x, line, tile_pixel);
	}
}

void ppu_render_sprites(ppu_t* ppu, mmu_t* mmu, size_t line)
{
	for (size_t i_sprite = 0; i_sprite < 40; i_sprite++)
	{
		uint16_t sprite_address = i_sprite * 4;
		uint8_t sprite_y = mmu->oam[sprite_address + 0];
		uint8_t sprite_x = mmu->oam[sprite_address + 1];
		uint8_t sprite_tile_id = mmu->oam[sprite_address + 2];
		uint8_t sprite_tile_attributes = mmu->oam[sprite_address + 3];

		/* check sprite is active */
		if (sprite_y)
		{
			uint8_t sprite_height = (mmu->io[MMAP_IO_LCDC - 0xFF00] & 0x4) ? 16 : 8;
			if (line >= (sprite_y - 16) && line < (sprite_y - 16 + sprite_height))
			{
				for (size_t x = 0; x < 8; x++)
				{
					size_t screen_x = sprite_x + x - 8;
					size_t screen_y = line;

					uint8_t pixel_x = screen_x % 8;
					uint8_t pixel_y = screen_y % sprite_height;

					uint8_t pixel = ppu_get_tile(ppu, mmu, sprite_tile_id, pixel_x, pixel_y, (sprite_tile_attributes & 0x10) ? mmu->io[0x49] : mmu->io[0x48], true);
					if (pixel != 0xBC) ppu_set_pixel(ppu, sprite_x + x - 8, line, pixel);
				}
			}
		}
	}
}

void ppu_render_line(ppu_t* ppu, mmu_t* mmu, size_t line)
{
	if (mmu->io[MMAP_IO_LCDC - 0xFF00] & 0x80)
	{
		if (mmu->io[MMAP_IO_LCDC - 0xFF00] & 0x1)
		{
			ppu_render_background(ppu, mmu, line);
		}
		if (mmu->io[MMAP_IO_LCDC - 0xFF00] & 0x2)
		{
			ppu_render_sprites(ppu, mmu, line);
		}
	}
}
