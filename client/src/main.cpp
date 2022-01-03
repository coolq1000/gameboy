
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <deque>
#include <core/dmg.hpp>
#include <SFML/Graphics.hpp>
#include "audio.hpp"

constexpr auto lcd_width = 160;
constexpr auto lcd_height = 144;

constexpr auto window_width = lcd_width * 4;
constexpr auto window_height = lcd_height * 4;

const char* cart_path = "../../res/roms/la.gb";
const char* save_path = "../../res/roms/la.sav";
//  const char* save_path = "";

namespace app
{
	sf::RenderWindow window;
	sf::Texture lcd;
	sf::Sprite lcd_sprite;
	sf::Font font;
	sf::Text text;
	sf::View view;
	std::unique_ptr<uint32_t> lcd_pixels;

	gmb_c::dmg_t gameboy;
	gmb_c::rom_t rom;

    audio_stream as(reinterpret_cast<gmb::apu*>(&gameboy.apu));

	void draw();

	void start()
	{
		gmb_c::rom_init(&rom, cart_path, save_path);
		gmb_c::dmg_init(&gameboy, &rom, false);

		window.create(sf::VideoMode(window_width, window_height), "gameboy");
		bool created = lcd.create(lcd_width, lcd_height);

		lcd_sprite = sf::Sprite(lcd);
		auto scale_x = window_width / lcd_width;
		auto scale_y = window_height / lcd_height;
		lcd_sprite.scale(scale_x, scale_y);

		view.setSize(window_width, window_height);
		view.setCenter(window_width / 2.0f, window_height / 2.0f);

		lcd_pixels = std::unique_ptr<uint32_t>(new uint32_t[lcd_width * lcd_height]());

		window.setFramerateLimit(60);

        as.play();
	}

	void stop()
	{
		gmb_c::rom_dump_save(&rom, &gameboy.mmu, save_path);
		gmb_c::rom_free(&rom);
	}

    void set_turbo(bool turbo)
    {
        if (turbo)
        {
            gameboy.ppu.frame_step = 5;
            window.setFramerateLimit(0);
        }
        else
        {
            gameboy.ppu.frame_step = 1;
            window.setFramerateLimit(60);
        }
    }

	void run()
	{
        usize i = 0;

		while (window.isOpen())
		{
			gmb_c::dmg_cycle(&gameboy);
            if ((gameboy.cpu.interrupt.master && gameboy.ppu.draw) || i % 1000000 == 0) draw();

            i++;
		}
	}

	float lerp(float a, float b, float t)
	{
		return a + t * (b - a);
	}

	void draw()
	{
		/* reposition lcd sprite */
		sf::Vector2u window_size = window.getSize();
		float window_ratio = window_size.x / (float)window_size.y;
		float lcd_ratio = lcd_width / (float)lcd_height;
		
		float pos_x = 0, pos_y = 0;
		float size_x = 0, size_y = 0;

		bool horizontal_spacing = window_ratio >= lcd_ratio;

		if (horizontal_spacing)
		{
			size_x = lcd_ratio / window_ratio;
			pos_x = (1.0f - size_x) / 2.0f;
		}
		else
		{
			size_y = window_ratio / lcd_ratio;
			pos_y = (1.0f - size_y) / 2.0f;
		}

		sf::Event event;
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::Resized:
				window.setView(view);
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

                case sf::Keyboard::Space: set_turbo(true); break;
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

                case sf::Keyboard::Space: set_turbo(false); break;
				}
				break;
			}
		}

		for (size_t x = 0; x < lcd_width; x++)
		{
			for (size_t y = 0; y < lcd_height; y++)
			{
				uint32_t raw_pixel = gmb_c::ppu_get_pixel(&gameboy.ppu, x, y);
				// lcd_pixels.get()[x + (y * 160)] = raw_pixel;

				const static float saturation = 0.95f;
				const static float gamma = 1.6f;

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

		window.clear();

		lcd.update(reinterpret_cast<sf::Uint8*>(lcd_pixels.get()));

		view.setViewport(sf::FloatRect(pos_x, pos_y, size_x, size_y));

		window.draw(lcd_sprite);

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
