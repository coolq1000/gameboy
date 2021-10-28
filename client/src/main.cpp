
#include <iostream>

#include <core/dmg.h>

int main()
{
	rom_t rom;
	rom_create(&rom, "roms/tetris.gb");

	dmg_t dmg;
	dmg_create(&dmg, &rom);
	std::cout << "Hello, World!" << std::endl;
	return EXIT_SUCCESS;
}
