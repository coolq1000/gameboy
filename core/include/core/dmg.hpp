
#ifndef DMG_HPP
#define DMG_HPP

#include <string>
#include "util.h"

namespace gmb_c
{
	extern "C"
	{
        #include "apu.h"
		#include "cpu.h"
		#include "dmg.h"
		#include "mmu.h"
		#include "opc.h"
		#include "ppu.h"
		#include "rom.h"
	}
}

namespace gmb
{
    struct DMG;

    struct APU {
        gmb_c::dmg_t& core;

        usize& sample_rate;
        usize& latency;

        APU(gmb_c::dmg_t& core)
            : core(core)
            , sample_rate(core.apu.sample_rate)
            , latency(core.apu.latency) {}
    };

    struct MMU {
        gmb_c::dmg_t& core;

        MMU(gmb_c::dmg_t& core) : core(core) {}
    };

    struct PPU {
        gmb_c::dmg_t& core;

        PPU(gmb_c::dmg_t& core) : core(core) {}
    };

    struct ROM
    {
        const std::string& cart_path, save_path;

        gmb_c::rom_t core_rom;

        ROM(const std::string& _cart_path, const std::string& _save_path)
            : cart_path(_cart_path)
            , save_path(_save_path) {
            gmb_c::rom_init(
                    &core_rom,
                    _cart_path.c_str(),
                    _save_path.c_str());
        }

        ~ROM() {
            gmb_c::rom_free(&core_rom);
        }

        void dump_save(MMU& mmu) {
            gmb_c::rom_dump_save(&core_rom,
                                 &mmu.core.mmu,
                                 save_path.c_str());
        }
    };

	struct DMG {
        gmb_c::dmg_t core;

        APU apu;
        MMU mmu;
        PPU ppu;

        DMG(ROM& rom, bool is_cgb, usize sample_rate, usize latency)
            : apu(core)
            , mmu(core)
            , ppu(core) {
            gmb_c::dmg_init(
                    &core,
                    &rom.core_rom, is_cgb,
                    sample_rate,
                    latency);
        }

        ~DMG() {
            gmb_c::dmg_free(&core);
        }

        void cycle() {
            gmb_c::dmg_cycle(&core);
        }
    };
}

#endif
