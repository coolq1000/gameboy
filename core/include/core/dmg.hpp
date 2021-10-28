
#ifndef DMG_HPP
#define DMG_HPP

// namespace gmb
// {
	extern "C"
	{
		#include "cpu.h"
		#include "dmg.h"
		#include "mmu.h"
		#include "opc.h"
		#include "ppu.h"
		#include "rom.h"
	}
// }

// namespace dmg
// {
// 	class cpu
// 	{
// 	public:

// 		gmb::cpu_t obj;

// 		inline cpu()
// 		{
// 			gmb::cpu_create(&obj);
// 		}

// 		inline ~cpu()
// 		{
// 			gmb::cpu_destroy(&obj);
// 		}
// 	};

// 	class rom
// 	{
// 	public:

// 		gmb::rom_t obj;

// 		inline rom()
// 		{
// 			gmb::rom_create(&obj);
// 		}

// 		inline ~rom()
// 		{
// 			gmb::rom_destroy(&obj);
// 		}
// 	};

// 	class mmu
// 	{
// 	public:

// 		gmb::mmu_t obj;

// 		inline mmu()
// 		{
// 			gmb::mmu_create(&obj);
// 		}

// 		inline ~mmu()
// 		{
// 			gmb::mmu_destroy(&obj);
// 		}
// 	};

// 	class ppu
// 	{
// 	public:

// 		gmb::ppu_t obj;

// 		inline ppu()
// 		{
// 			gmb::ppu_create(&obj);
// 		}

// 		inline ~ppu()
// 		{
// 			gmb::ppu_destroy(&obj);
// 		}
// 	};

// 	class dmg
// 	{
// 	public:

// 		gmb::dmg_t obj;

// 		inline dmg(rom& cart)
// 		{
// 			gmb::dmg_create(&obj, &cart);
// 		}

// 		inline ~dmg()
// 		{
// 			gmb::dmg_destroy(&obj);
// 		}
// 	};
// }

#endif
