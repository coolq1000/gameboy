
#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "mmu.h"
#include "opc.h"

#define IS_ZERO(reg) (!reg)
#define HALF_CARRY(a, b) (((a + b) & 0x10) > 0)
#define FULL_CARRY(a, b) (((a + b) & 0x100) > 0)

typedef struct cpu
{
	struct
	{
		/* assumes little-endian */
		union
		{
			struct
			{
				union
				{
					struct
					{
						uint8_t flag__ : 4;
						uint8_t flag_c : 1;
						uint8_t flag_h : 1;
						uint8_t flag_n : 1;
						uint8_t flag_z : 1;
					};
					uint8_t f;
				};
				uint8_t a;
			};
			uint16_t af;
		};
		union { struct { uint8_t c; uint8_t b; }; uint16_t bc; };
		union { struct { uint8_t e; uint8_t d; }; uint16_t de; };
		union { struct { uint8_t l; uint8_t h; }; uint16_t hl; };

		uint16_t sp, pc;
	} registers;
	struct
	{
		size_t cycles;
	} clock;
	struct
	{
		bool master;
	} interrupt;
} cpu_t;

void cpu_create(cpu_t* cpu);
void cpu_destroy(cpu_t* cpu);

void cpu_fault(cpu_t* cpu, opc_t* opc, const char* message);
void cpu_trace(cpu_t* cpu, opc_t* opc);
void cpu_dump(cpu_t* cpu);
void cpu_cycle(cpu_t* cpu, mmu_t* mmu);

#endif
