
#ifndef ROM_H
#define ROM_H

#include <stdint.h>
#include <stddef.h>

#define ROM_TITLE_LENGTH 16
#define ROM_MANUFACTURER_LENGTH 4
#define ROM_LICENSE_LENGTH 2

typedef struct
{
	/* header data */
	struct
	{
		char title[ROM_TITLE_LENGTH + 1];
		char manufacturer[ROM_MANUFACTURER_LENGTH + 1];
		char cgb;
		char license[ROM_LICENSE_LENGTH + 1];
		char sgb;
		char type;
		char rom_size;
		char ram_size;
		char destination;
	} header;

	/* cart data */
	uint8_t* cart_data;
	size_t cart_size;

	/* save data */
	uint8_t* save_data;
	size_t save_size;
} rom_t;

void rom_create(rom_t* rom, const char* cart_path, const char* save_path);
void rom_destroy(rom_t* rom);

void rom_load_cart(rom_t* rom, const char* cart_path);
void rom_load_save(rom_t* rom, const char* save_path);

void rom_dump_save(rom_t* rom, void* mmu, const char* save_path);

#endif
