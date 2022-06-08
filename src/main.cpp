
#include <iostream>
#include <cstdlib>
#include <memory>
#include <vector>
#include <filesystem>
#include <core/dmg.hpp>
#include "window.hpp"
#include "audio.hpp"

struct Gameboy {
    gmb::ROM rom;
    gmb::DMG dmg;

    Window window;
    Audio audio_stream;

    std::vector<i16> sample_buffer;

    bool turbo_active = false;

    Gameboy(const std::string& cart_path, const std::string& save_path, bool is_cgb)
        : rom(cart_path, save_path)
        , dmg(rom, is_cgb, 48000, 2048)
        , window(dmg.ppu)
        , audio_stream(dmg.apu) {
        sample_buffer = std::vector<i16>(dmg.apu.latency * AUDIO_CHANNELS);
	}

	~Gameboy() {
		rom.dump_save(dmg.mmu);
	}

    void set_turbo(bool turbo) {
        turbo_active = turbo;

        if (turbo) {
            dmg.core.ppu.frame_step = SPEED_SHIFT;
        } else {
            dmg.core.ppu.frame_step = 1;
        }
    }

	void run() {
		while (window.open()) {
            dmg.cycle();
            if (dmg.core.apu.update) audio();
            if (dmg.core.ppu.draw) draw();
		}
	}

	void draw() {
        window.process();

        if (window.focused()) {
            dmg.core.mmu.buttons.up = window.get_key(SDL_SCANCODE_UP);
            dmg.core.mmu.buttons.down = window.get_key(SDL_SCANCODE_DOWN);
            dmg.core.mmu.buttons.left = window.get_key(SDL_SCANCODE_LEFT);
            dmg.core.mmu.buttons.right = window.get_key(SDL_SCANCODE_RIGHT);
            dmg.core.mmu.buttons.b = window.get_key(SDL_SCANCODE_Z);
            dmg.core.mmu.buttons.a = window.get_key(SDL_SCANCODE_X);
            dmg.core.mmu.buttons.start = window.get_key(SDL_SCANCODE_RETURN);
            dmg.core.mmu.buttons.select = window.get_key(SDL_SCANCODE_BACKSPACE);
            dmg.core.mmu.buttons.turbo = window.get_key(SDL_SCANCODE_SPACE);

            set_turbo(dmg.core.mmu.buttons.turbo);
        }

        window.update();
	}

    void audio() {
        static float buffer_fill = 0;
        sample_buffer[static_cast<usize>(buffer_fill * 2) + 0] = dmg.core.apu.output_left * 4;
        sample_buffer[static_cast<usize>(buffer_fill * 2) + 1] = dmg.core.apu.output_right * 4;

        buffer_fill += 1.0f / (turbo_active ? SPEED_SHIFT : 1.0f);

        if (buffer_fill >= dmg.apu.latency) {
            buffer_fill = 0;
            while (audio_stream.queued() > dmg.apu.latency * AUDIO_CHANNELS) {}
            audio_stream.queue(&sample_buffer[0], dmg.apu.latency * AUDIO_CHANNELS);
        }
    }
};

int main(int argc, char* argv[])
{
    std::filesystem::path cart_path, save_path;
    bool is_cgb = false;

    if (argc == 2) {
        cart_path = std::string(argv[1]);
        save_path = std::filesystem::path(std::string(argv[1]))
                .replace_extension(".sav");
        is_cgb = cart_path.extension().string().back() == 'c';
    } else {
        std::cerr << "[!] usage: gameboy <rom_path>" << std::endl;
        return EXIT_FAILURE;
    }

	Gameboy gb(cart_path.string(), save_path.string(), is_cgb);
    gb.run();
	return EXIT_SUCCESS;
}
