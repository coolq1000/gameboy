
#include <stdio.h>
#include <stdlib.h>

extern "C"
{
	#include <core/dmg.h>
}

#define ROM_FILE_PATH "roms/tetris.gb"

int main()
{
	bool debugging = false;

	/* load rom */
	rom_t rom;
	rom_create(&rom, ROM_FILE_PATH);

	printf("[+] loaded rom `%s`\n", ROM_FILE_PATH);
	printf("    [title] = %s\n", rom.header.title);
	printf("    [license] = %s\n", rom.header.license);
	printf("    [manufacturer_code] = %s\n", rom.header.manufacturer);

	/* load dmg core */
	dmg_t dmg;
	dmg_create(&dmg, &rom);

	/* cycle the cpu */
	for (size_t i = 0; i < 612860; i++)
	{
		opc_t* opcode = &opc_opcodes[mmu_peek8(&dmg.mmu, dmg.cpu.registers.pc)];
		if (dmg.cpu.registers.pc == 0x27D6)
		{
			debugging = true;
			break;
		}

		if (debugging)
		{
			cpu_trace(&dmg.cpu, opcode);
			cpu_dump(&dmg.cpu);
			cpu_stack_trace(&dmg.cpu, &dmg.mmu);
			printf(">: ");
			getchar();
		}

		dmg_cycle(&dmg);
	}

	cpu_dump(&dmg.cpu);
	cpu_stack_trace(&dmg.cpu, &dmg.mmu);

	for (int y = 0; y < 8; y++)
	{
		uint8_t line1 = dmg.mmu.vram[(y * 2) + 0];
		uint8_t line2 = dmg.mmu.vram[(y * 2) + 1];
		for (int x = 0; x < 8; x++)
		{
			uint8_t pixel_bit_1 = line1 & 0x1;
			uint8_t pixel_bit_2 = line2 & 0x1;
			uint8_t pixel = pixel_bit_1 | (pixel_bit_2 << 1);
			printf("%02X ", pixel);
			line1 >>= 1;
			line2 >>= 1;
		}
		printf("\n");
	}

	dmg_destroy(&dmg);
	return EXIT_SUCCESS;
}
