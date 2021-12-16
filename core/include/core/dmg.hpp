
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
        gmb_c::apu_t core_apu;

    public:

        inline apu()
        {
            gmb_c::apu_create(&core_apu);
        }

        inline ~apu()
        {
            gmb_c::apu_destroy(&core_apu);
        }

        inline i16 emit(u32 seek, usize sample_rate)
        {
            return core_apu.ch1.sample_callback(seek, sample_rate);
        }
    };

	// class cpu;

	// class dmg
	// {
	// 	cpu processor;
	// 	mmu memory;
	// 	ppu graphics;
	// };
}

#endif
