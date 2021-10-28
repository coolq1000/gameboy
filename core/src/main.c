
#include <stdio.h>
#include "core/dmg.h"

#define ROM_FILE_PATH "roms/tetris.gb"

int main_old()
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
	for (size_t i = 0; i < 100000; i++)
	{
		dmg_cycle(&dmg);
	}

	dmg_destroy(&dmg);
	return 0;
}
