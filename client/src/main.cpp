
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <core/dmg.hpp>
#include <SFML/Graphics.hpp>
#include "audio.hpp"

constexpr auto lcd_width = 160;
constexpr auto lcd_height = 144;

constexpr auto window_width = lcd_width * 4;
constexpr auto window_height = lcd_height * 4;

const char* cart_path = "../../res/roms/zs.gbc";
//const char* save_path = "../../res/roms/zs.sav";
  const char* save_path = "";

namespace app
{
	sf::RenderWindow window;
	sf::Texture lcd;
	sf::Sprite lcd_sprite;
	sf::Font font;
	sf::Text text;
	std::unique_ptr<uint32_t> lcd_pixels;

    gmb::rom rom(cart_path, save_path);
    gmb::dmg gameboy(rom, true, 48000, 1024);

    audio as(gameboy.apu_);

	void draw();

	void start()
	{
		window.create(sf::VideoMode(window_width, window_height), "gameboy");
		bool created = lcd.create(lcd_width, lcd_height);

		lcd_sprite = sf::Sprite(lcd);

		lcd_pixels = std::unique_ptr<uint32_t>(new uint32_t[lcd_width * lcd_height]());

		window.setFramerateLimit(60);
	}

	void stop()
	{
		rom.dump_save(gameboy.mmu_);
	}

    void set_turbo(bool turbo)
    {
        if (turbo)
        {
            gameboy.core_dmg.ppu.frame_step = 5;
            window.setFramerateLimit(0);
        }
        else
        {
            gameboy.core_dmg.ppu.frame_step = 1;
            window.setFramerateLimit(60);
        }
    }

	void run()
	{
        usize i = 0;

		while (window.isOpen())
		{
            gameboy.cycle();
            if ((gameboy.core_dmg.cpu.interrupt.master && gameboy.core_dmg.ppu.draw) || i % 1000000 == 0) draw();
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
		float lcd_ratio = (float)lcd_width / (float)lcd_height;

        float pos_x = 0.0f;
        float pos_y = 0.0f;
        float size_x = 1.0f;
        float size_y = 1.0f;

        if (window_ratio > lcd_ratio)
        {
            size_x = lcd_ratio / window_ratio;
            pos_x = (1.0f - size_x) / 2.0f;
        }
        else
        {
            size_y = window_ratio / lcd_ratio;
            pos_y = (1.0f - size_y) / 2.0f;
        }

        float scale_x = (float)window_width / (float)lcd_width;
        float scale_y = (float)window_height / (float)lcd_height;

        lcd_sprite.setScale(size_x * scale_x, size_y * scale_y);
        lcd_sprite.setPosition(pos_x * window_width, pos_y * window_height);

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
				case sf::Keyboard::Enter: gameboy.core_dmg.mmu.buttons.start = true; break;
				case sf::Keyboard::Backspace: gameboy.core_dmg.mmu.buttons.select = true; break;
				case sf::Keyboard::Z: gameboy.core_dmg.mmu.buttons.b = true; break;
				case sf::Keyboard::X: gameboy.core_dmg.mmu.buttons.a = true; break;
				case sf::Keyboard::Down: gameboy.core_dmg.mmu.buttons.down = true; break;
				case sf::Keyboard::Up: gameboy.core_dmg.mmu.buttons.up = true; break;
				case sf::Keyboard::Left: gameboy.core_dmg.mmu.buttons.left = true; break;
				case sf::Keyboard::Right: gameboy.core_dmg.mmu.buttons.right = true; break;

                case sf::Keyboard::Space: set_turbo(true); break;
				}
				break;
			case sf::Event::KeyReleased:
				switch (event.key.code)
				{
				case sf::Keyboard::Enter: gameboy.core_dmg.mmu.buttons.start = false; break;
				case sf::Keyboard::Backspace: gameboy.core_dmg.mmu.buttons.select = false; break;
				case sf::Keyboard::Z: gameboy.core_dmg.mmu.buttons.b = false; break;
				case sf::Keyboard::X: gameboy.core_dmg.mmu.buttons.a = false; break;
				case sf::Keyboard::Down: gameboy.core_dmg.mmu.buttons.down = false; break;
				case sf::Keyboard::Up: gameboy.core_dmg.mmu.buttons.up = false; break;
				case sf::Keyboard::Left: gameboy.core_dmg.mmu.buttons.left = false; break;
				case sf::Keyboard::Right: gameboy.core_dmg.mmu.buttons.right = false; break;

                case sf::Keyboard::Space: set_turbo(false); break;
				}
				break;
			}
		}

		for (size_t x = 0; x < lcd_width; x++)
		{
			for (size_t y = 0; y < lcd_height; y++)
			{
				uint32_t raw_pixel = gmb_c::ppu_get_pixel(&gameboy.core_dmg.ppu, x, y);
				// lcd_pixels.get()[x + (y * 160)] = raw_pixel;

				const static float saturation = 0.95f;
                const static float brightness = 0.05f;
				const static float gamma = 1.6f;

				uint8_t a = 0xFF;
				uint8_t r = (raw_pixel >> 16) & 0xFF;
				uint8_t g = (raw_pixel >> 8) & 0xFF;
				uint8_t b = (raw_pixel >> 0) & 0xFF;

				float r_f = ((float)r) / 0xFF;
				float g_f = ((float)g) / 0xFF;
				float b_f = ((float)b) / 0xFF;

				float luma = (r_f + g_f + b_f) / 3.0f;

				r_f = lerp(luma, r_f, saturation) + (luma ? brightness : 0.0f);
				g_f = lerp(luma, g_f, saturation) + (luma ? brightness : 0.0f);
				b_f = lerp(luma, b_f, saturation) + (luma ? brightness : 0.0f);

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
