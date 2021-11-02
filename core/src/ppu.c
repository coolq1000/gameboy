#include "core/ppu.h"

#include <stdio.h>

void ppu_create(ppu_t* ppu)
{
	ppu->mode = MODE_OAM;
	ppu->cycles = 0;
	ppu->line = 0; // todo: check this
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

void ppu_render_background(ppu_t* ppu, mmu_t* mmu, size_t line)
{
	for (size_t x = 0; x < LCD_WIDTH; x++)
	{
		size_t tile_x = x / 8;
		size_t tile_y = line / 8;
		uint8_t tile_id = mmu_peek8(mmu, 0x9800 + tile_x + tile_y * 32);
		uint16_t tile_addr = tile_id * 8 * 2;

		uint8_t pixel_x = x % 8;
		uint8_t pixel_y = line % 8;

		uint8_t pixel_line_1 = mmu->vram[(tile_addr + pixel_y * 2) + 0];
		uint8_t pixel_line_2 = mmu->vram[(tile_addr + pixel_y * 2) + 1];

		uint8_t pixel_mask = 0x80 >> pixel_x;
		uint8_t pixel = (((pixel_line_1 & pixel_mask) != 0) << 1) | ((pixel_line_2 & pixel_mask) != 0);

		ppu_set_pixel(ppu, x, line, pixel);
	}
}

void ppu_render_sprites(ppu_t* ppu, mmu_t* mmu, size_t line)
{

}

void ppu_render_line(ppu_t* ppu, mmu_t* mmu, size_t line)
{
	ppu_render_background(ppu, mmu, line);
}
