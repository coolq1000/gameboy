#include "core/rom.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "core/mmu.h"

#define ROM_TITLE_OFFSET 0x134
#define ROM_MANUFACTURER_OFFSET 0x13F
#define ROM_LICENSE_OFFSET 0x144

void rom_create(rom_t* rom, const char* cart_path, const char* save_path)
{
	/* zero out data before loading */
	rom->cart_data = NULL;
	rom->save_data = NULL;
	rom->cart_size = 0;
	rom->save_size = 0;

	/* load cartridge & save */
	rom_load_cart(rom, cart_path);
	rom_load_save(rom, save_path);
}

void rom_destroy(rom_t* rom)
{
	if (rom->cart_data) free(rom->cart_data);
	if (rom->save_data) free(rom->save_data);
}

void rom_load_cart(rom_t* rom, const char* cart_path)
{
	/* free if existing */
	if (rom->cart_data) { free(rom->cart_data); }

	/* load rom from file */
	FILE* rom_file = fopen(cart_path, "rb");

	if (rom_file)
	{
		/* calculate file size */
		fseek(rom_file, 0, SEEK_END);
		rom->cart_size = ftell(rom_file);
		rewind(rom_file);

		/* allocate & read into buffer */
		rom->cart_data = (uint8_t*)malloc(rom->cart_size);
		fread(rom->cart_data, rom->cart_size, sizeof(uint8_t), rom_file);

		fclose(rom_file);

		/* parse rom */
		memcpy(rom->header.title, &rom->cart_data[ROM_TITLE_OFFSET], ROM_TITLE_LENGTH);
		memcpy(rom->header.manufacturer, &rom->cart_data[ROM_MANUFACTURER_OFFSET], ROM_MANUFACTURER_LENGTH);
		memcpy(rom->header.license, &rom->cart_data[ROM_LICENSE_OFFSET], ROM_LICENSE_LENGTH);
	}
	else
	{
		printf("[!] unable to read rom file at `%s`\n", cart_path);
		exit(EXIT_FAILURE);
	}
}

void rom_load_save(rom_t* rom, const char* save_path)
{
	/* free if existing */
	if (rom->save_data) { free(rom->save_data); }

	/* load save from file */
	FILE* save_file = fopen(save_path, "rb");

	if (save_file)
	{
		/* calculate file size */
		fseek(save_file, 0, SEEK_END);
		rom->save_size = ftell(save_file);
		rewind(save_file);

		/* allocate & read into buffer */
		rom->save_data = (uint8_t*)malloc(rom->save_size);
		fread(rom->save_data, rom->save_size, sizeof(uint8_t), save_file);

		fclose(save_file);
	}
	else
	{
		printf("[-] unable to read rom save file at `%s`\n", save_path);
	}
}

void rom_dump_save(rom_t* rom, void* mmu, const char* save_path)
{
	mmu_t* mmu_ = (mmu_t*)mmu;
	if (*save_path)
	{
		/* load save from file */
		FILE* save_file = fopen(save_path, "wb");

		if (save_file)
		{
			/* write out xram buffer */
			fwrite(mmu_->xram[0], sizeof(uint8_t), XRAM_SIZE, save_file); // todo: dump all buffers

			fclose(save_file);
		}
		else
		{
			printf("[-] unable to write rom save file at `%s`\n", save_path);
		}
	}
}
