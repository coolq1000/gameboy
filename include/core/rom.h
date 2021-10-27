
#ifndef ROM_H
#define ROM_H

#include <stdint.h>

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

	/* rom data */
	uint8_t* data;
	size_t size;
} rom_t;

void rom_create(rom_t* rom, const char* path);
void rom_destroy(rom_t* rom);

#endif
