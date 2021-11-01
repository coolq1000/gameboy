#include "core/ppu.h"

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
	ppu->line = (ppu->line + 1) % LINE_MAX;
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

			if (ppu->line == LINE_V_BLANK)
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
			// todo: render scanline
			ppu->mode = MODE_H_BLANK;
			// todo: hdma transfer
			// todo: set lcd-transfer stat bit
			ppu->cycles -= CYCLES_LCD_TRANSFER;
		}
		break;
	case MODE_V_BLANK:
		if (CYCLES_ELAPSED(ppu, CYCLES_LINE))
		{
			ppu_update_ly(ppu, mmu);
			// todo: check ly == lyc
			ppu->cycles -= CYCLES_LINE;
		}
		break;
	}
}
