
#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "mmu.h"
#include "opc.h"

/* flags */
#define IS_ZERO(reg) (!(reg))
#define HALF_CARRY(a, b) ((((a) + (b)) & 0x10) > 0)
#define FULL_CARRY(a, b) ((((a) + (b)) & 0x100) > 0)
#define DOUBLE_HALF_CARRY(a, b) ((((a) + (b)) & 0x100) > 0)
#define DOUBLE_FULL_CARRY(a, b) ((((a) + (b)) & 0x1000) > 0)

/* interrupts */
#define IRF 0xFF0F /* interrupt request flag */

#define INT_V_BLANK		0x40
#define INT_LCD_STAT	0x48
#define INT_TIMER		0x50
#define INT_SERIAL		0x58
#define INT_JOYPAD		0x60

#define INT_V_BLANK_INDEX	(1 << 0)
#define INT_LCD_STAT_INDEX	(1 << 1)
#define INT_TIMER_INDEX		(1 << 2)
#define INT_SERIAL_INDEX	(1 << 3)
#define INT_JOYPAD_INDEX	(1 << 4)

#define INT_TIMER_DIV	0xFF04
#define INT_TIMER_TIMA	0xFF05
#define INT_TIMER_TMA	0xFF06
#define INT_TIMER_TAC	0xFF07

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
		uint8_t cycles;
		uint16_t div_diff;
		uint8_t tima_mod;
	} clock;
	struct
	{
		int pending;
		bool master;
	} interrupt;

	bool halted;
} cpu_t;

void cpu_create(cpu_t* cpu);
void cpu_destroy(cpu_t* cpu);

void cpu_fault(cpu_t* cpu, mmu_t* mmu, opc_t* opc, const char* message);
void cpu_trace(cpu_t* cpu, opc_t* opc);
void cpu_stack_trace(cpu_t* cpu, mmu_t* mmu);
void cpu_dump(cpu_t* cpu);
void cpu_call(cpu_t* cpu, mmu_t* mmu, uint16_t address);
void cpu_ret(cpu_t* cpu, mmu_t* mmu);
void cpu_execute(cpu_t* cpu, mmu_t* mmu, uint8_t opcode);
void cpu_execute_cb(cpu_t* cpu, mmu_t* mmu, uint8_t opcode);
void cpu_request(cpu_t* cpu, mmu_t* mmu, uint8_t index);
void cpu_interrupt(cpu_t* cpu, mmu_t* mmu, uint16_t address);
void cpu_cycle(cpu_t* cpu, mmu_t* mmu);

#endif
