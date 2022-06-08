
#include <cstdlib>
#include <memory>
#include <vector>
#include <core/dmg.hpp>
#include "window.hpp"
#include "audio.hpp"

const char* cart_path = "../../res/roms/la.gb";
const char* save_path = "../../res/roms/la.sav";
//const char* save_path = "";

namespace app
{
    gmb::rom rom(cart_path, save_path);
    gmb::dmg gameboy(rom, false, 48000, 2048);

    window win(gameboy.ppu_);
    audio as(gameboy.apu_);

    std::vector<i16> sample_buffer;

    bool turbo_active = false;

	void draw();
    void audio();

	void start()
	{
        sample_buffer = std::vector<i16>(gameboy.apu_.latency * AUDIO_CHANNELS);
	}

	void stop()
	{
		rom.dump_save(gameboy.mmu_);
	}

    void set_turbo(bool turbo)
    {
        turbo_active = turbo;

        if (turbo)
        {
            gameboy.core_dmg.ppu.frame_step = SPEED_SHIFT;
        }
        else
        {
            gameboy.core_dmg.ppu.frame_step = 1;
        }
    }

	void run()
	{
		while (win.open())
		{
            gameboy.cycle();
            if (gameboy.core_dmg.apu.update) audio();
            if (gameboy.core_dmg.ppu.draw) draw();
		}
	}

	void draw()
	{
        win.process();

        if (win.focused())
        {
            gameboy.core_dmg.mmu.buttons.up = win.get_key(SDL_SCANCODE_UP);
            gameboy.core_dmg.mmu.buttons.down = win.get_key(SDL_SCANCODE_DOWN);
            gameboy.core_dmg.mmu.buttons.left = win.get_key(SDL_SCANCODE_LEFT);
            gameboy.core_dmg.mmu.buttons.right = win.get_key(SDL_SCANCODE_RIGHT);
            gameboy.core_dmg.mmu.buttons.b = win.get_key(SDL_SCANCODE_Z);
            gameboy.core_dmg.mmu.buttons.a = win.get_key(SDL_SCANCODE_X);
            gameboy.core_dmg.mmu.buttons.start = win.get_key(SDL_SCANCODE_RETURN);
            gameboy.core_dmg.mmu.buttons.select = win.get_key(SDL_SCANCODE_BACKSPACE);
            gameboy.core_dmg.mmu.buttons.turbo = win.get_key(SDL_SCANCODE_SPACE);

            set_turbo(gameboy.core_dmg.mmu.buttons.turbo);
        }

        win.update();
	}

    void audio()
    {
        static float buffer_fill = 0;
        sample_buffer[static_cast<usize>(buffer_fill * 2) + 0] = gameboy.core_dmg.apu.output_left * 4;
        sample_buffer[static_cast<usize>(buffer_fill * 2) + 1] = gameboy.core_dmg.apu.output_right * 4;

        buffer_fill += 1.0f / (turbo_active ? SPEED_SHIFT : 1.0f);

        if (buffer_fill >= gameboy.apu_.latency)
        {
            buffer_fill = 0;
            while (as.queued() > gameboy.apu_.latency * AUDIO_CHANNELS) {}
            as.queue(&sample_buffer[0], gameboy.apu_.latency * AUDIO_CHANNELS);
        }
    }
}

int main()
{
	app::start();
	app::run();
	app::stop();

	return EXIT_SUCCESS;
}
