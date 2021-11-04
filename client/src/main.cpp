
#include <stdio.h>
#include <stdlib.h>
#include <deque>
#include <core/dmg.hpp>
#include <SFML/Graphics.hpp>

constexpr auto lcd_width = 160;
constexpr auto lcd_height = 144;

constexpr auto window_width = lcd_width * 4;
constexpr auto window_height = lcd_height * 4;

namespace app
{
	sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Gameboy");
	sf::Texture lcd;
	sf::Sprite lcd_sprite;
	std::unique_ptr<uint32_t> lcd_pixels;

	gmb_c::dmg_t gameboy;

	void draw();

	void start()
	{
		gmb_c::rom_t rom;
		gmb_c::rom_create(&rom, "roms/tetris.gb");
		gmb_c::dmg_create(&gameboy, &rom);

		lcd.create(lcd_width, lcd_height);

		lcd_sprite = sf::Sprite(lcd);
		auto scale_x = window_width / lcd_width;
		auto scale_y = window_height / lcd_height;
		lcd_sprite.scale(scale_x, scale_y);

		lcd_pixels = std::unique_ptr<uint32_t>(new uint32_t[lcd_width * lcd_height]());

		gameboy.ppu.v_blank_callback = draw;
	}

	void run()
	{
		while (window.isOpen())
		{
			for (size_t i_cycle = 0; i_cycle < 6; i_cycle++)
			{
				gmb_c::dmg_cycle(&gameboy);
			}

			gameboy.mmu.buttons.start = sf::Keyboard::isKeyPressed(sf::Keyboard::Enter);
			gameboy.mmu.buttons.select = sf::Keyboard::isKeyPressed(sf::Keyboard::Backspace);
			gameboy.mmu.buttons.a = sf::Keyboard::isKeyPressed(sf::Keyboard::Z);
			gameboy.mmu.buttons.b = sf::Keyboard::isKeyPressed(sf::Keyboard::X);
			gameboy.mmu.buttons.down = sf::Keyboard::isKeyPressed(sf::Keyboard::Down);
			gameboy.mmu.buttons.up = sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
			gameboy.mmu.buttons.left = sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
			gameboy.mmu.buttons.right = sf::Keyboard::isKeyPressed(sf::Keyboard::Right);
		}
	}

	void draw()
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

		for (size_t x = 0; x < lcd_width; x++)
		{
			for (size_t y = 0; y < lcd_height; y++)
			{
				uint8_t pixel = gmb_c::ppu_get_pixel(&gameboy.ppu, x, y);
				uint8_t r, g, b, a;
				r = 0xFF - (pixel * 68);
				g = 0xFF - (pixel * 73);
				b = 0xFF - (pixel * 73);
				a = 0xFF;

				lcd_pixels.get()[x + (y * 160)] = (a << 24) | (b << 16) | (g << 8) | r;
			}
		}

		lcd.update(reinterpret_cast<sf::Uint8*>(lcd_pixels.get()));

		window.draw(lcd_sprite);
		window.display();
	}
};

int main()
{
	app::start();
	app::run();

	return EXIT_SUCCESS;
}
