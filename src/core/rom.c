#include "core/rom.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROM_TITLE_OFFSET 0x134
#define ROM_MANUFACTURER_OFFSET 0x13F
#define ROM_LICENSE_OFFSET 0x144

void rom_create(rom_t* rom, const char* path)
{
	/* load rom from file */
	FILE* rom_file = fopen(path, "rb");

	if (rom_file)
	{
		/* calculate file size */
		fseek(rom_file, 0, SEEK_END);
		rom->size = ftell(rom_file);
		rewind(rom_file);

		/* allocate & read into buffer */
		rom->data = (uint8_t*)malloc(rom->size);
		fread(rom->data, rom->size, sizeof(uint8_t), rom_file);

		fclose(rom_file);

		/* parse rom */
		memcpy(rom->header.title, &rom->data[ROM_TITLE_OFFSET], ROM_TITLE_LENGTH);
		memcpy(rom->header.manufacturer, &rom->data[ROM_MANUFACTURER_OFFSET], ROM_MANUFACTURER_LENGTH);
		memcpy(rom->header.license, &rom->data[ROM_LICENSE_OFFSET], ROM_LICENSE_LENGTH);
	}
	else
	{
		printf("[!] unable to read rom file at `%s`\n", path);
		exit(EXIT_FAILURE);
	}
}

void rom_destroy(rom_t* rom)
{
	free(rom->data);
}
