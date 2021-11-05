
#include <stdio.h>
#include <stdlib.h>
#include <core/dmg.hpp>
#include <SFML/Graphics.hpp>

constexpr auto lcd_width = 160;
constexpr auto lcd_height = 144;

constexpr auto window_width = lcd_width * 4;
constexpr auto window_height = lcd_height * 4;

namespace app
{
	sf::RenderWindow window;
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

		window.create(sf::VideoMode(window_width, window_height), "gameboy");

		lcd.create(lcd_width, lcd_height);

		lcd_sprite = sf::Sprite(lcd);
		auto scale_x = window_width / lcd_width;
		auto scale_y = window_height / lcd_height;
		lcd_sprite.scale(scale_x, scale_y);

		lcd_pixels = std::unique_ptr<uint32_t>(new uint32_t[lcd_width * lcd_height]());

		window.setFramerateLimit(60);

		gameboy.ppu.v_blank_callback = draw;
	}

	void run()
	{
		while (window.isOpen())
		{
			gmb_c::dmg_cycle(&gameboy);
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
			case sf::Event::KeyPressed:
				switch (event.key.code)
				{
				case sf::Keyboard::Enter: gameboy.mmu.buttons.start = true; break;
				case sf::Keyboard::Backspace: gameboy.mmu.buttons.select = true; break;
				case sf::Keyboard::Z: gameboy.mmu.buttons.a = true; break;
				case sf::Keyboard::X: gameboy.mmu.buttons.b = true; break;
				case sf::Keyboard::Down: gameboy.mmu.buttons.down = true; break;
				case sf::Keyboard::Up: gameboy.mmu.buttons.up = true; break;
				case sf::Keyboard::Left: gameboy.mmu.buttons.left = true; break;
				case sf::Keyboard::Right: gameboy.mmu.buttons.right = true; break;
				}
				break;
			case sf::Event::KeyReleased:
				switch (event.key.code)
				{
				case sf::Keyboard::Enter: gameboy.mmu.buttons.start = false; break;
				case sf::Keyboard::Backspace: gameboy.mmu.buttons.select = false; break;
				case sf::Keyboard::Z: gameboy.mmu.buttons.a = false; break;
				case sf::Keyboard::X: gameboy.mmu.buttons.b = false; break;
				case sf::Keyboard::Down: gameboy.mmu.buttons.down = false; break;
				case sf::Keyboard::Up: gameboy.mmu.buttons.up = false; break;
				case sf::Keyboard::Left: gameboy.mmu.buttons.left = false; break;
				case sf::Keyboard::Right: gameboy.mmu.buttons.right = false; break;
				}
				break;
			}
		}

		for (size_t x = 0; x < lcd_width; x++)
		{
			for (size_t y = 0; y < lcd_height; y++)
			{
				uint8_t pixel = gmb_c::ppu_get_pixel(&gameboy.ppu, x, y);
				uint8_t r = 0, g = 0, b = 0, a = 0xFF;

				r = gmb_c::ppu_palette[pixel][0];
				g = gmb_c::ppu_palette[pixel][1];
				b = gmb_c::ppu_palette[pixel][2];

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
