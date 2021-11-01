
#ifndef DMG_HPP
#define DMG_HPP

#include <string>

namespace gmb_c
{
	extern "C"
	{
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
	class rom_t
	{
	public:

		gmb_c::rom_t obj;

		inline rom_t(const std::string&& path)
		{
			gmb_c::rom_create(&obj, path.c_str());
		}

		inline ~rom_t()
		{
			gmb_c::rom_destroy(&obj);
		}
	};

	class dmg_t
	{
	public:

		gmb_c::dmg_t obj;

		gmb_c::cpu_t& cpu;
		gmb_c::mmu_t& mmu;
		gmb_c::ppu_t& ppu;

		inline dmg_t(rom_t&& cart)
			:
			cpu(obj.cpu),
			ppu(obj.ppu),
			mmu(obj.mmu)
		{
			gmb_c::dmg_create(&obj, &cart.obj);
		}

		inline ~dmg_t()
		{
			gmb_c::dmg_destroy(&obj);
		}

		inline void cycle()
		{
			gmb_c::dmg_cycle(&obj);
		}
	};
}

#endif
