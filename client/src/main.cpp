
#include <stdio.h>
#include <stdlib.h>
#include <core/dmg.hpp>
#include <SFML/Graphics.hpp>

constexpr auto lcd_width = 160;
constexpr auto lcd_height = 144;

constexpr auto window_width = lcd_width * 4;
constexpr auto window_height = lcd_height * 4;

int main()
{
	sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Gameboy");

	sf::Texture lcd;
	lcd.create(lcd_width, lcd_height);

	sf::Sprite lcd_sprite(lcd);
	auto scale_x = window_width / lcd_width;
	auto scale_y = window_height / lcd_height;
	lcd_sprite.scale(scale_x, scale_y);

	auto lcd_pixels = std::unique_ptr<uint32_t>(new uint32_t[lcd_width * lcd_height]());

	gmb::dmg_t gameboy(gmb::rom_t("roms/tetris.gb"));

	for (size_t i = 0; i < 98129; i++)
	{
		gameboy.cycle();
	}

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				window.close();
				break;
			}
		}

		for (int y = 0; y < 8; y++)
		{
			uint8_t line1 = gameboy.mmu.vram[(y * 2) + 0];
			uint8_t line2 = gameboy.mmu.vram[(y * 2) + 1];
			for (int x = 0; x < 8; x++)
			{
				uint8_t pixel_bit_1 = line1 & 0x1;
				uint8_t pixel_bit_2 = line2 & 0x1;
				uint8_t pixel = pixel_bit_1 | (pixel_bit_2 << 1);
				lcd_pixels.get()[x + (y * 160)] = 0xFF0000FF;
				line1 >>= 1;
				line2 >>= 1;
			}
			printf("\n");
		}

		lcd.update(reinterpret_cast<sf::Uint8*>(lcd_pixels.get()));

		window.draw(lcd_sprite);
		window.display();
	}

	printf("%llX\n", gmb_c::mmu_peek8(&gameboy.mmu, 0x9820));

	return EXIT_SUCCESS;
}
