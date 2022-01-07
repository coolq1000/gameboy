
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <core/dmg.hpp>
#include "window.hpp"
#include "audio.hpp"

const char* cart_path = "../../res/roms/za.gbc";
const char* save_path = "../../res/roms/za.sav";
//  const char* save_path = "";

namespace app
{


    gmb::rom rom(cart_path, save_path);
    gmb::dmg gameboy(rom, true, 48000, 2048);

    window win(gameboy.ppu_);
    audio as(gameboy.apu_);

	void draw();

	void start()
	{

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
            win.set_frame_rate(0);
        }
        else
        {
            gameboy.core_dmg.ppu.frame_step = 1;
            win.set_frame_rate(60);
        }
    }

	void run()
	{
        usize i = 0;

		while (win.open())
		{
            gameboy.cycle();
            if ((gameboy.core_dmg.cpu.interrupt.master && gameboy.core_dmg.ppu.draw) || i % 1000000 == 0) draw();
            i++;
		}
	}

	void draw()
	{
        win.process();

        gameboy.core_dmg.mmu.buttons.up = win.get_key(SDL_SCANCODE_UP);
        gameboy.core_dmg.mmu.buttons.down = win.get_key(SDL_SCANCODE_DOWN);
        gameboy.core_dmg.mmu.buttons.left = win.get_key(SDL_SCANCODE_LEFT);
        gameboy.core_dmg.mmu.buttons.right = win.get_key(SDL_SCANCODE_RIGHT);
        gameboy.core_dmg.mmu.buttons.b = win.get_key(SDL_SCANCODE_Z);
        gameboy.core_dmg.mmu.buttons.a = win.get_key(SDL_SCANCODE_X);
        gameboy.core_dmg.mmu.buttons.start = win.get_key(SDL_SCANCODE_RETURN);
        gameboy.core_dmg.mmu.buttons.select = win.get_key(SDL_SCANCODE_BACKSPACE);

        win.update();
	}
};

int main()
{
	app::start();
	app::run();
	app::stop();

	return EXIT_SUCCESS;
}
