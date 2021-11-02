
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

		inline dmg_t(rom_t&& cart)
		{
			gmb_c::dmg_create(&obj, &cart.obj);
		}

		inline ~dmg_t()
		{
			gmb_c::dmg_destroy(&obj);
		}

		template<typename T>
		inline T peek(uint16_t address)
		{
			T data{};
			for (size_t i = 0; i < sizeof(T); i++)
			{
				*(reinterpret_cast<uint8_t*>(&data) + i) = gmb_c::mmu_peek8(&obj.mmu, address + i);
			}

			return data;
		}

		template<typename T>
		inline void poke(uint16_t address, T value)
		{
			for (size_t i = 0; i < sizeof(T); i++)
			{
				gmb_c::mmu_poke8(&obj.mmu, address + i, *(reinterpret_cast<uint8_t*>(&value) + i));
			}
		}

		inline void cycle()
		{
			gmb_c::dmg_cycle(&obj);
		}
	};
}

#endif
