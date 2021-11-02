
#include <stdio.h>
#include <stdlib.h>
#include <core/dmg.hpp>
#include <SFML/Graphics.hpp>

constexpr auto lcd_width = 160;
constexpr auto lcd_height = 144;

constexpr auto window_width = lcd_width * 4;
constexpr auto window_height = lcd_height * 4;

class application_t
{
	sf::RenderWindow window;
	sf::Texture lcd;
	sf::Sprite lcd_sprite;
	std::unique_ptr<uint32_t> lcd_pixels;

	gmb::dmg_t gameboy;

public:

	application_t()
		:
		window(sf::VideoMode(window_width, window_height), "Gameboy"),
		gameboy(gmb::rom_t("roms/tetris.gb"))
	{
		lcd.create(lcd_width, lcd_height);

		lcd_sprite = sf::Sprite(lcd);
		auto scale_x = window_width / lcd_width;
		auto scale_y = window_height / lcd_height;
		lcd_sprite.scale(scale_x, scale_y);

		lcd_pixels = std::unique_ptr<uint32_t>(new uint32_t[lcd_width * lcd_height]());
	}

	void run()
	{
		for (size_t i = 0; i < 30001840; i++)
		{
			gameboy.cycle();
		}

		// gmb_c::cpu_dump(&gameboy.obj.cpu);

		while (window.isOpen())
		{
			// gmb_c::ppu_cycle(&gameboy.ppu, &gameboy.mmu, &gameboy.cpu, 4);
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

			for (size_t x = 0; x < lcd_width; x++)
			{
				for (size_t y = 0; y < lcd_height; y++)
				{
					uint8_t pixel = gmb_c::ppu_get_pixel(&gameboy.obj.ppu, x, y);
					uint8_t r, g, b, a;
					r = 0xFF - (pixel * 85);
					g = 0xFF - (pixel * 85);
					b = 0xFF - (pixel * 85);
					a = 0xFF;

					lcd_pixels.get()[x + (y * 160)] = (a << 24) | (b << 16) | (g << 8) | r;
				}
			}

			lcd.update(reinterpret_cast<sf::Uint8*>(lcd_pixels.get()));

			window.draw(lcd_sprite);
			window.display();
		}
	}
};

int main()
{
	application_t app;
	app.run();
	//0x9820
	return EXIT_SUCCESS;
}
