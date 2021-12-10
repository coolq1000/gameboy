
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <deque>
#include <sstream>
#include <core/dmg.hpp>
#include <SFML/Graphics.hpp>

constexpr auto lcd_width = 160;
constexpr auto lcd_height = 144;

constexpr auto window_width = lcd_width * 4;
constexpr auto window_height = lcd_height * 4;

const char* cart_path = "res/roms/zs.gbc";
// const char* save_path = "res/roms/poke_red.sav";
const char* save_path = "";

namespace app
{
	sf::RenderWindow window;
	sf::Texture lcd;
	sf::Sprite lcd_sprite;
	sf::Font font;
	sf::Text text;
	std::unique_ptr<uint32_t> lcd_pixels;

	std::deque<gmb_c::cpu> history;

	gmb_c::dmg_t gameboy;
	gmb_c::rom_t rom;

	void draw();

	void start()
	{
		gmb_c::rom_create(&rom, cart_path, save_path);
		gmb_c::dmg_create(&gameboy, &rom, true);

		window.create(sf::VideoMode(window_width, window_height), "gameboy");

		lcd.create(lcd_width, lcd_height);

		lcd_sprite = sf::Sprite(lcd);
		auto scale_x = window_width / lcd_width;
		auto scale_y = window_height / lcd_height;
		lcd_sprite.scale(scale_x, scale_y);

		lcd_pixels = std::unique_ptr<uint32_t>(new uint32_t[lcd_width * lcd_height]());

		window.setFramerateLimit(60);

		gameboy.ppu.v_blank_callback = draw;

		font.loadFromFile("res/fonts/slkscr.ttf");
		text.setFont(font);
		text.setFillColor(sf::Color(200, 200, 200, 255));
        text.setOutlineColor(sf::Color(25, 25, 25, 255));
        text.setOutlineThickness(2);
        
        std::atexit([]()
    	{
    		for (auto& history_element : history)
    		{
    			gmb_c::cpu_dump(&history_element);
    		}
    	});
	}

	void stop()
	{
		gmb_c::rom_dump_save(&rom, &gameboy.mmu, save_path);
		gmb_c::rom_destroy(&rom);
	}

	void run()
	{
		bool debugging = false;
		while (window.isOpen())
		{
			// history.push_front(gameboy.cpu);
			// while (history.size() > 64) history.pop_back();
			if (gameboy.cpu.registers.pc == 0x407A)// && gmb_c::mmu_peek8(&gameboy.mmu, gameboy.cpu.registers.pc) == 0x22)
			{
				// debugging = true;
			}
			if (debugging)
			{
				gmb_c::opc_t* opcode = &gmb_c::opc_opcodes[gmb_c::mmu_peek8(&gameboy.mmu, gameboy.cpu.registers.pc)];
				gmb_c::cpu_trace(&gameboy.cpu, opcode);
				gmb_c::cpu_dump(&gameboy.cpu);
				printf(">: ");
				getchar();
			}
			gmb_c::dmg_cycle(&gameboy);
		}
	}

	float lerp(float a, float b, float t)
	{
		return a + t * (b - a);
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
				case sf::Keyboard::Z: gameboy.mmu.buttons.b = true; break;
				case sf::Keyboard::X: gameboy.mmu.buttons.a = true; break;
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
				case sf::Keyboard::Z: gameboy.mmu.buttons.b = false; break;
				case sf::Keyboard::X: gameboy.mmu.buttons.a = false; break;
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
				uint32_t raw_pixel = gmb_c::ppu_get_pixel(&gameboy.ppu, x, y);

				const static float saturation = 0.85f;
				const static float gamma = 1.5f;

				uint8_t a = 0xFF;
				uint8_t r = (raw_pixel >> 16) & 0xFF;
				uint8_t g = (raw_pixel >> 8) & 0xFF;
				uint8_t b = (raw_pixel >> 0) & 0xFF;

				float r_f = ((float)r) / 0xFF;
				float g_f = ((float)g) / 0xFF;
				float b_f = ((float)b) / 0xFF;

				float luma = (r_f + g_f + b_f) / 3.0f;

				r_f = lerp(luma, r_f, saturation);
				g_f = lerp(luma, g_f, saturation);
				b_f = lerp(luma, b_f, saturation);

				r_f = std::pow(r_f, 1.0f / gamma);
				g_f = std::pow(g_f, 1.0f / gamma);
				b_f = std::pow(b_f, 1.0f / gamma);

				r = std::min(std::max(r_f, 0.0f), 1.0f) * 0xFF;
				g = std::min(std::max(g_f, 0.0f), 1.0f) * 0xFF;
				b = std::min(std::max(b_f, 0.0f), 1.0f) * 0xFF;

				lcd_pixels.get()[x + (y * 160)] = (a << 24) | (r << 16) | (g << 8) | b;
			}
		}

		lcd.update(reinterpret_cast<sf::Uint8*>(lcd_pixels.get()));

		window.draw(lcd_sprite);

		// text.setString(std::to_string(gmb_c::mmu_peek8(&gameboy.mmu, MMAP_IO_STAT)));
		// text.setCharacterSize(24);
		// window.draw(text);

		window.display();
	}
};

int main()
{
	app::start();
	app::run();
	app::stop();

	return EXIT_SUCCESS;
}
