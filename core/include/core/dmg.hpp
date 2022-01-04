
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
    class apu
    {
    public:

        gmb_c::apu_t& core_apu;

        usize& sample_rate;
        usize& latency;

        apu(gmb_c::apu_t& _apu) : core_apu(_apu), sample_rate(_apu.sample_rate), latency(_apu.latency)
        {}

        void update()
        {
            gmb_c::apu_update(&core_apu);
        }
    };

    class mmu
    {
    public:

        gmb_c::mmu_t& core_mmu;

        mmu(gmb_c::mmu_t& _mmu) : core_mmu(_mmu)
        {}
    };

    class rom
    {
        const std::string& cart_path, save_path;

    public:

        gmb_c::rom_t core_rom;

        rom(const std::string& _cart_path, const std::string& _save_path) : cart_path(_cart_path), save_path(_save_path)
        {
            gmb_c::rom_init(&core_rom, _cart_path.c_str(), _save_path.c_str());
        }

        ~rom()
        {
            gmb_c::rom_free(&core_rom);
        }

        void dump_save(mmu& _mmu)
        {
            gmb_c::rom_dump_save(&core_rom, &_mmu.core_mmu, save_path.c_str());
        }
    };

	class dmg
    {
    public:

        gmb_c::dmg_t core_dmg;

        apu apu_;
        mmu mmu_;

        dmg(rom& _rom, bool _is_cgb, usize _sample_rate, usize _latency) : apu_(core_dmg.apu), mmu_(core_dmg.mmu)
        {
            gmb_c::dmg_init(&core_dmg, &_rom.core_rom, _is_cgb, _sample_rate, _latency);
        }

        ~dmg()
        {
            gmb_c::dmg_free(&core_dmg);
        }

        void cycle()
        {
            gmb_c::dmg_cycle(&core_dmg);
            apu_.update();
        }
    };
}

#endif
