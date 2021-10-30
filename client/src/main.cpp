
#include <stdio.h>
#include <stdlib.h>
#include <core/dmg.hpp>

#define ROM_FILE_PATH "roms/tetris.gb"

int main()
{
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
	for (size_t i = 0; i < 10000000; i++)
	{
		dmg_cycle(&dmg);
	}

	for (size_t i = 0; i < 100; i++)
	{
		dmg_cycle(&dmg);
		cpu_dump(&dmg.cpu);
	}

	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			printf("%X ", dmg.mmu.vram[x + (y * 8)]);
		}
		printf("\n");
	}

	dmg_destroy(&dmg);
	return EXIT_SUCCESS;
}
