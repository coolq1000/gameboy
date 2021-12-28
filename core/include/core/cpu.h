
#ifndef CPU_H
#define CPU_H

#include "opc.h"
#include "bus.h"
#include "util.h"

/* flags */
#define IS_ZERO(reg) (!(reg))

/* interrupts */
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
						u8 flag__ : 4;
						u8 flag_c : 1;
						u8 flag_h : 1;
						u8 flag_n : 1;
						u8 flag_z : 1;
					};
					u8 f;
				};
				u8 a;
			};
			u16 af;
		};
		union { struct { u8 c; u8 b; }; u16 bc; };
		union { struct { u8 e; u8 d; }; u16 de; };
		union { struct { u8 l; u8 h; }; u16 hl; };

		u16 sp, pc;
	} registers;
	struct
	{
		u8 cycles;
		u16 div_diff;
		u8 tima_mod;
	} clock;
	struct
	{
		usize pending;
		bool master;
	} interrupt;
	struct
	{
		bool enabled;
	} cgb;

	bool stopped;
	bool halted;
} cpu_t;

void cpu_init(cpu_t* cpu, bool is_cgb);

void cpu_fault(cpu_t* cpu, bus_t* bus, opc_t* opc, const char* message);
void cpu_trace(cpu_t* cpu, opc_t* opc);
void cpu_stack_trace(cpu_t* cpu, bus_t* bus);
void cpu_dump(cpu_t* cpu);
void cpu_call(cpu_t* cpu, bus_t* bus, u16 address);
void cpu_ret(cpu_t* cpu, bus_t* bus);
void cpu_execute(cpu_t* cpu, bus_t* bus, u8 opcode);
void cpu_execute_cb(cpu_t* cpu, bus_t* bus, u8 opcode);
void cpu_request(cpu_t* cpu, bus_t* bus, u8 index);
void cpu_interrupt(cpu_t* cpu, bus_t* bus, u16 address);
void cpu_cycle(cpu_t* cpu, bus_t* bus);

#endif
