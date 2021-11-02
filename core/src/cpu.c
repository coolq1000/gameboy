#include "core/cpu.h"

#include <stdio.h>
#include <stdlib.h>

void cpu_create(cpu_t* cpu)
{
	/* setup registers */
	cpu->registers.af = 0x01B0;
	cpu->registers.bc = 0x0013;
	cpu->registers.de = 0x00D8;
	cpu->registers.hl = 0x014D;

	cpu->registers.sp = 0xFFFE;
	cpu->registers.pc = 0x0100;

	/* setup clock */
	cpu->clock.cycles = 0;
	cpu->clock.div_diff = 0;
	cpu->clock.tima_mod = 0;

	/* setup interrupts */
	cpu->interrupt.delay = 0;
	cpu->interrupt.master = true;

	/* halt */
	cpu->halted = false;
}

void cpu_destroy(cpu_t* cpu)
{
	
}

void cpu_fault(cpu_t* cpu, mmu_t* mmu, opc_t* opc, const char* message)
{
	/* reverse opcode state differential */
	cpu->registers.pc -= opc->length;

	/* output fault */
	cpu_trace(cpu, opc);
	printf("            ^~~~ fault received `%s`\n", message);
	printf("[!] encountered fatal cpu exception\n");
	printf("	-- core dump --\n");
	cpu_dump(cpu);
	printf("	-- stack trace --\n");
	cpu_stack_trace(cpu, mmu);
	exit(EXIT_FAILURE);
}

void cpu_trace(cpu_t* cpu, opc_t* opc)
{
	printf("[+] 0x%X: %s\n", cpu->registers.pc, opc->disasm);
}

void cpu_stack_trace(cpu_t* cpu, mmu_t* mmu)
{
	for (size_t i = 0; i < 9; i++)
	{
		uint16_t address = (cpu->registers.sp - (i * 2)) + (4 * 2);
		printf("%s %04X: %04X\n", cpu->registers.sp == address ? ">" : " ", address, mmu_peek16(mmu, address));
	}
}

void cpu_dump(cpu_t* cpu)
{
	printf("	af: %04X\n", cpu->registers.af);
	printf("	bc: %04X\n", cpu->registers.bc);
	printf("	de: %04X\n", cpu->registers.de);
	printf("	hl: %04X\n", cpu->registers.hl);
	printf("	sp: %04X\n", cpu->registers.sp);
	printf("	pc: %04X\n", cpu->registers.pc);
	printf("   ime: %s\n", cpu->interrupt.master ? "enabled" : "disabled");
}

void cpu_push(cpu_t* cpu, mmu_t* mmu, uint16_t value)
{
	/* decrement stack */
	cpu->registers.sp -= sizeof(uint16_t);

	/* write value onto stack */
	mmu_poke16(mmu, cpu->registers.sp, value);
}

uint16_t cpu_pop(cpu_t* cpu, mmu_t* mmu)
{
	/* read value from stack */
	uint16_t value = mmu_peek16(mmu, cpu->registers.sp);

	/* increment stack */
	cpu->registers.sp += sizeof(uint16_t);

	return value;
}

void cpu_call(cpu_t* cpu, mmu_t* mmu, uint16_t address)
{
	cpu_push(cpu, mmu, cpu->registers.pc);
	cpu->registers.pc = address;
}

void cpu_ret(cpu_t* cpu, mmu_t* mmu)
{
	cpu->registers.pc = cpu_pop(cpu, mmu);
}

void cpu_execute(cpu_t* cpu, mmu_t* mmu, uint8_t opcode)
{
	/* decode opcode & immediate values */
	opc_t* opc = &opc_opcodes[opcode];
	uint8_t imm8 = mmu_peek8(mmu, cpu->registers.pc + 1);
	uint16_t imm16 = mmu_peek16(mmu, cpu->registers.pc + 1);

	/* update state */
	cpu->registers.pc += opc->length;
	cpu->clock.cycles = opc->cycles;

	/* temporary values */
	uint8_t tmp8;
	uint16_t tmp16;

	/* execute based on opcode */
	switch (opcode)
	{
	case 0x00: /* nop */
		break;
	case 0x01: /* ld bc, d16 */
		cpu->registers.bc = imm16;
		break;
	case 0x02: /* ld (bc), a */
		mmu_poke8(mmu, cpu->registers.bc, cpu->registers.a);
		break;
	case 0x03: /* inc bc */
		cpu->registers.bc++;
		break;
	case 0x04: /* inc b */
		cpu->registers.flag_h = HALF_CARRY(cpu->registers.b, 1);
		cpu->registers.flag_z = IS_ZERO(++cpu->registers.b);
		cpu->registers.flag_n = 0;
		break;
	case 0x05: /* dec b */
		cpu->registers.flag_h = HALF_CARRY(cpu->registers.b, -1);
		cpu->registers.flag_z = IS_ZERO(--cpu->registers.b);
		cpu->registers.flag_n = 1;
		break;
	case 0x06: /* ld b, d8 */
		cpu->registers.b = imm8;
		break;
	case 0x07: /* rlca */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x08: /* ld (a16), sp */
		mmu_poke16(mmu, imm16, cpu->registers.sp);
		break;
	case 0x09: /* add hl, bc */
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = HALF_CARRY(cpu->registers.hl, cpu->registers.bc);
		cpu->registers.flag_c = FULL_CARRY(cpu->registers.hl, cpu->registers.bc);
		cpu->registers.hl += cpu->registers.bc;
		break;
	case 0x0A: /* ld a, (bc) */
		cpu->registers.a = mmu_peek8(mmu, cpu->registers.bc);
		break;
	case 0x0B: /* dec bc */
		cpu->registers.bc--;
		break;
	case 0x0C: /* inc c */
		cpu->registers.flag_h = HALF_CARRY(cpu->registers.c, 1);
		cpu->registers.flag_z = IS_ZERO(++cpu->registers.c);
		cpu->registers.flag_n = 0;
		break;
	case 0x0D: /* dec c */
		cpu->registers.flag_h = HALF_CARRY(cpu->registers.c, -1);
		cpu->registers.flag_z = IS_ZERO(--cpu->registers.c);
		cpu->registers.flag_n = 1;
		break;
	case 0x0E: /* ld c, d8 */
		cpu->registers.c = imm8;
		break;
	case 0x0F: /* rrca */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x10: /* stop 0 */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x11: /* ld de, d16 */
		cpu->registers.de = imm16;
		break;
	case 0x12: /* ld (de), a */
		mmu_poke8(mmu, cpu->registers.de, cpu->registers.a);
		break;
	case 0x13: /* inc de */
		cpu->registers.de++;
		break;
	case 0x14: /* inc d */
		cpu->registers.flag_h = HALF_CARRY(cpu->registers.d, 1);
		cpu->registers.flag_z = IS_ZERO(++cpu->registers.d);
		cpu->registers.flag_n = 0;
		break;
	case 0x15: /* dec d */
		cpu->registers.flag_h = HALF_CARRY(cpu->registers.d, -1);
		cpu->registers.flag_z = IS_ZERO(--cpu->registers.d);
		cpu->registers.flag_n = 1;
		break;
	case 0x16: /* ld d, d8 */
		cpu->registers.d = imm8;
		break;
	case 0x17: /* rla */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x18: /* jr r8 */
		cpu->registers.pc += (int8_t)imm8;
		break;
	case 0x19: /* add hl, de */
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = DOUBLE_HALF_CARRY(cpu->registers.hl, cpu->registers.de);
		cpu->registers.flag_h = DOUBLE_FULL_CARRY(cpu->registers.hl, cpu->registers.de);
		cpu->registers.hl += cpu->registers.de;
		break;
	case 0x1A: /* ld a, (de) */
		cpu->registers.a = mmu_peek8(mmu, cpu->registers.de);
		break;
	case 0x1B: /* dec de */
		cpu->registers.de--;
		break;
	case 0x1C: /* inc e */
		cpu->registers.flag_h = HALF_CARRY(cpu->registers.e, 1);
		cpu->registers.flag_z = IS_ZERO(++cpu->registers.e);
		cpu->registers.flag_n = 0;
		break;
	case 0x1D: /* dec e */
		cpu->registers.flag_h = HALF_CARRY(cpu->registers.e, -1);
		cpu->registers.flag_z = IS_ZERO(--cpu->registers.e);
		cpu->registers.flag_n = 1;
		break;
	case 0x1E: /* ld e, d8 */
		cpu->registers.e = imm8;
		break;
	case 0x1F: /* rra */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x20: /* jr nz, r8 */
		if (!cpu->registers.flag_z)
		{
			cpu->registers.pc += (int8_t)imm8;
			cpu->clock.cycles += 4;
		}
		break;
	case 0x21: /* ld hl, d16 */
		cpu->registers.hl = imm16;
		break;
	case 0x22: /* ld (hl+), a */
		mmu_poke8(mmu, cpu->registers.hl++, cpu->registers.a);
		break;
	case 0x23: /* inc hl */
		cpu->registers.hl++;
		break;
	case 0x24: /* inc h */
		cpu->registers.flag_h = HALF_CARRY(cpu->registers.h, 1);
		cpu->registers.flag_z = IS_ZERO(++cpu->registers.h);
		cpu->registers.flag_n = 0;
		break;
	case 0x25: /* dec h */
		cpu->registers.flag_h = HALF_CARRY(cpu->registers.h, -1);
		cpu->registers.flag_z = IS_ZERO(--cpu->registers.h);
		cpu->registers.flag_n = 1;
		break;
	case 0x26: /* ld h, d8 */
		cpu->registers.h = imm8;
		break;
	case 0x27: /* daa */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x28: /* jr z, r8 */
		if (cpu->registers.flag_z)
		{
			cpu->registers.pc += (int8_t)imm8;
			cpu->clock.cycles += 4;
		}
		break;
	case 0x29: /* add hl, hl */
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = DOUBLE_HALF_CARRY(cpu->registers.hl, cpu->registers.hl);
		cpu->registers.flag_h = DOUBLE_FULL_CARRY(cpu->registers.hl, cpu->registers.hl);
		cpu->registers.hl += cpu->registers.hl;
		break;
	case 0x2A: /* ld a, (hl+) */
		cpu->registers.a = mmu_peek8(mmu, cpu->registers.hl++);
		break;
	case 0x2B: /* dec hl */
		cpu->registers.hl--;
		break;
	case 0x2C: /* inc l */
		cpu->registers.flag_h = HALF_CARRY(cpu->registers.l, 1);
		cpu->registers.flag_z = IS_ZERO(++cpu->registers.l);
		cpu->registers.flag_n = 0;
		break;
	case 0x2D: /* dec l */
		cpu->registers.flag_h = HALF_CARRY(cpu->registers.l, -1);
		cpu->registers.flag_z = IS_ZERO(--cpu->registers.l);
		cpu->registers.flag_n = 1;
		break;
	case 0x2E: /* ld l, d8 */
		cpu->registers.l = imm8;
		break;
	case 0x2F: /* cpl */
		cpu->registers.a = ~cpu->registers.a;
		cpu->registers.flag_n = true;
		cpu->registers.flag_h = true;
		break;
	case 0x30: /* jr nc, r8 */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x31: /* ld sp, d16 */
		cpu->registers.sp = imm16;
		break;
	case 0x32: /* ld (hl-), a */
		mmu_poke8(mmu, cpu->registers.hl--, cpu->registers.a);
		break;
	case 0x33: /* inc sp */
		cpu->registers.sp++;
		break;
	case 0x34: /* inc (hl) */
		tmp8 = mmu_peek8(mmu, cpu->registers.hl);
		cpu->registers.flag_h = HALF_CARRY(tmp8, 1);
		cpu->registers.flag_z = IS_ZERO(++tmp8);
		cpu->registers.flag_n = 1;
		mmu_poke8(mmu, cpu->registers.hl, tmp8);
		break;
	case 0x35: /* dec (hl) */
		tmp8 = mmu_peek8(mmu, cpu->registers.hl);
		cpu->registers.flag_h = HALF_CARRY(tmp8, -1);
		cpu->registers.flag_z = IS_ZERO(--tmp8);
		cpu->registers.flag_n = 1;
		mmu_poke8(mmu, cpu->registers.hl, tmp8);
		break;
	case 0x36: /* ld (hl), d8 */
		mmu_poke8(mmu, cpu->registers.hl, imm8);
		break;
	case 0x37: /* scf */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x38: /* jr c, r8 */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x39: /* add hl, sp */
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = DOUBLE_HALF_CARRY(cpu->registers.hl, cpu->registers.sp);
		cpu->registers.flag_h = DOUBLE_FULL_CARRY(cpu->registers.hl, cpu->registers.sp);
		cpu->registers.hl += cpu->registers.sp;
		break;
	case 0x3A: /* ld a, (hl-) */
		cpu->registers.a = mmu_peek8(mmu, cpu->registers.hl--);
		break;
	case 0x3B: /* dec sp */
		cpu->registers.sp--;
		break;
	case 0x3C: /* inc a */
		cpu->registers.flag_h = HALF_CARRY(cpu->registers.a, 1);
		cpu->registers.flag_z = IS_ZERO(++cpu->registers.a);
		cpu->registers.flag_n = 0;
		break;
	case 0x3D: /* dec a */
		cpu->registers.flag_h = HALF_CARRY(cpu->registers.a, -1);
		cpu->registers.flag_z = IS_ZERO(--cpu->registers.a);
		cpu->registers.flag_n = 1;
		break;
	case 0x3E: /* ld a, d8 */
		cpu->registers.a = imm8;
		break;
	case 0x3F: /* ccf */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x40: /* ld b, b */
		cpu->registers.b = cpu->registers.b;
		break;
	case 0x41: /* ld b, c */
		cpu->registers.b = cpu->registers.c;
		break;
	case 0x42: /* ld b, d */
		cpu->registers.b = cpu->registers.d;
		break;
	case 0x43: /* ld b, e */
		cpu->registers.b = cpu->registers.e;
		break;
	case 0x44: /* ld b, h */
		cpu->registers.b = cpu->registers.h;
		break;
	case 0x45: /* ld b, l */
		cpu->registers.b = cpu->registers.l;
		break;
	case 0x46: /* ld b, (hl) */
		cpu->registers.b = mmu_peek8(mmu, cpu->registers.hl);
		break;
	case 0x47: /* ld b, a */
		cpu->registers.b = cpu->registers.a;
		break;
	case 0x48: /* ld c, b */
		cpu->registers.c = cpu->registers.b;
		break;
	case 0x49: /* ld c, c */
		cpu->registers.c = cpu->registers.c;
		break;
	case 0x4A: /* ld c, d */
		cpu->registers.c = cpu->registers.d;
		break;
	case 0x4B: /* ld c, e */
		cpu->registers.c = cpu->registers.e;
		break;
	case 0x4C: /* ld c, h */
		cpu->registers.c = cpu->registers.h;
		break;
	case 0x4D: /* ld c, l */
		cpu->registers.c = cpu->registers.l;
		break;
	case 0x4E: /* ld c, (hl) */
		cpu->registers.c = mmu_peek8(mmu, cpu->registers.hl);
		break;
	case 0x4F: /* ld c, a */
		cpu->registers.c = cpu->registers.a;
		break;
	case 0x50: /* ld d, b */
		cpu->registers.d = cpu->registers.b;
		break;
	case 0x51: /* ld d, c */
		cpu->registers.d = cpu->registers.c;
		break;
	case 0x52: /* ld d, d */
		cpu->registers.d = cpu->registers.d;
		break;
	case 0x53: /* ld d, e */
		cpu->registers.d = cpu->registers.e;
		break;
	case 0x54: /* ld d, h */
		cpu->registers.d = cpu->registers.h;
		break;
	case 0x55: /* ld d, l */
		cpu->registers.d = cpu->registers.l;
		break;
	case 0x56: /* ld d, (hl) */
		cpu->registers.d = mmu_peek8(mmu, cpu->registers.hl);
		break;
	case 0x57: /* ld d, a */
		cpu->registers.d = cpu->registers.a;
		break;
	case 0x58: /* ld e, b */
		cpu->registers.e = cpu->registers.b;
		break;
	case 0x59: /* ld e, c */
		cpu->registers.e = cpu->registers.c;
		break;
	case 0x5A: /* ld e, d */
		cpu->registers.e = cpu->registers.d;
		break;
	case 0x5B: /* ld e, e */
		cpu->registers.e = cpu->registers.e;
		break;
	case 0x5C: /* ld e, h */
		cpu->registers.e = cpu->registers.h;
		break;
	case 0x5D: /* ld e, l */
		cpu->registers.e = cpu->registers.l;
		break;
	case 0x5E: /* ld e, (hl) */
		cpu->registers.e = mmu_peek8(mmu, cpu->registers.hl);
		break;
	case 0x5F: /* ld e, a */
		cpu->registers.e = cpu->registers.a;
		break;
	case 0x60: /* ld h, b */
		cpu->registers.h = cpu->registers.b;
		break;
	case 0x61: /* ld h, c */
		cpu->registers.h = cpu->registers.c;
		break;
	case 0x62: /* ld h, d */
		cpu->registers.h = cpu->registers.d;
		break;
	case 0x63: /* ld h, e */
		cpu->registers.h = cpu->registers.e;
		break;
	case 0x64: /* ld h, h */
		cpu->registers.h = cpu->registers.h;
		break;
	case 0x65: /* ld h, l */
		cpu->registers.h = cpu->registers.l;
		break;
	case 0x66: /* ld h, (hl) */
		cpu->registers.h = mmu_peek8(mmu, cpu->registers.hl);
		break;
	case 0x67: /* ld h, a */
		cpu->registers.h = cpu->registers.a;
		break;
	case 0x68: /* ld l, b */
		cpu->registers.l = cpu->registers.b;
		break;
	case 0x69: /* ld l, c */
		cpu->registers.l = cpu->registers.c;
		break;
	case 0x6A: /* ld l, d */
		cpu->registers.l = cpu->registers.d;
		break;
	case 0x6B: /* ld l, e */
		cpu->registers.l = cpu->registers.e;
		break;
	case 0x6C: /* ld l, h */
		cpu->registers.l = cpu->registers.h;
		break;
	case 0x6D: /* ld l, l */
		cpu->registers.l = cpu->registers.l;
		break;
	case 0x6E: /* ld l, (hl) */
		cpu->registers.l = mmu_peek8(mmu, cpu->registers.hl);
		break;
	case 0x6F: /* ld l, a */
		cpu->registers.l = cpu->registers.a;
		break;
	case 0x70: /* ld (hl), b */
		mmu_poke8(mmu, cpu->registers.hl, cpu->registers.b);
		break;
	case 0x71: /* ld (hl), c */
		mmu_poke8(mmu, cpu->registers.hl, cpu->registers.c);
		break;
	case 0x72: /* ld (hl), d */
		mmu_poke8(mmu, cpu->registers.hl, cpu->registers.d);
		break;
	case 0x73: /* ld (hl), e */
		mmu_poke8(mmu, cpu->registers.hl, cpu->registers.e);
		break;
	case 0x74: /* ld (hl), h */
		mmu_poke8(mmu, cpu->registers.hl, cpu->registers.h);
		break;
	case 0x75: /* ld (hl), l */
		mmu_poke8(mmu, cpu->registers.hl, cpu->registers.l);
		break;
	case 0x76: /* halt */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x77: /* ld (hl), a */
		mmu_poke8(mmu, cpu->registers.hl, cpu->registers.a);
		break;
	case 0x78: /* ld a, b */
		cpu->registers.a = cpu->registers.b;
		break;
	case 0x79: /* ld a, c */
		cpu->registers.a = cpu->registers.c;
		break;
	case 0x7A: /* ld a, d */
		cpu->registers.a = cpu->registers.d;
		break;
	case 0x7B: /* ld a, e */
		cpu->registers.a = cpu->registers.e;
		break;
	case 0x7C: /* ld a, h */
		cpu->registers.a = cpu->registers.h;
		break;
	case 0x7D: /* ld a, l */
		cpu->registers.a = cpu->registers.l;
		break;
	case 0x7E: /* ld a, (hl) */
		cpu->registers.a = mmu_peek8(mmu, cpu->registers.hl);
		break;
	case 0x7F: /* ld a, a */
		cpu->registers.a = cpu->registers.a;
		break;
	case 0x80: /* add a, b */
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = HALF_CARRY(cpu->registers.a, cpu->registers.b);
		cpu->registers.flag_h = FULL_CARRY(cpu->registers.a, cpu->registers.b);
		cpu->registers.a += cpu->registers.b;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		break;
	case 0x81: /* add a, c */
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = HALF_CARRY(cpu->registers.a, cpu->registers.c);
		cpu->registers.flag_h = FULL_CARRY(cpu->registers.a, cpu->registers.c);
		cpu->registers.a += cpu->registers.c;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.c);
		break;
	case 0x82: /* add a, d */
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = HALF_CARRY(cpu->registers.a, cpu->registers.d);
		cpu->registers.flag_h = FULL_CARRY(cpu->registers.a, cpu->registers.d);
		cpu->registers.a += cpu->registers.d;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		break;
	case 0x83: /* add a, e */
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = HALF_CARRY(cpu->registers.a, cpu->registers.e);
		cpu->registers.flag_h = FULL_CARRY(cpu->registers.a, cpu->registers.e);
		cpu->registers.a += cpu->registers.e;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		break;
	case 0x84: /* add a, h */
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = HALF_CARRY(cpu->registers.a, cpu->registers.h);
		cpu->registers.flag_h = FULL_CARRY(cpu->registers.a, cpu->registers.h);
		cpu->registers.a += cpu->registers.h;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		break;
	case 0x85: /* add a, l */
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = HALF_CARRY(cpu->registers.a, cpu->registers.l);
		cpu->registers.flag_h = FULL_CARRY(cpu->registers.a, cpu->registers.l);
		cpu->registers.a += cpu->registers.l;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		break;
	case 0x86: /* add a, (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x87: /* add a, a */
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = HALF_CARRY(cpu->registers.a, cpu->registers.a);
		cpu->registers.flag_h = FULL_CARRY(cpu->registers.a, cpu->registers.a);
		cpu->registers.a += cpu->registers.a;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		break;
	case 0x88: /* adc a, b */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x89: /* adc a, c */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x8A: /* adc a, d */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x8B: /* adc a, e */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x8C: /* adc a, h */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x8D: /* adc a, l */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x8E: /* adc a, (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x8F: /* adc a, a */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x90: /* sub b */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x91: /* sub c */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x92: /* sub d */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x93: /* sub e */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x94: /* sub h */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x95: /* sub l */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x96: /* sub (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x97: /* sub a */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x98: /* sbc a, b */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x99: /* sbc a, c */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x9A: /* sbc a, d */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x9B: /* sbc a, e */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x9C: /* sbc a, h */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x9D: /* sbc a, l */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x9E: /* sbc a, (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0x9F: /* sbc a, a */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0xA0: /* and b */
		cpu->registers.a &= cpu->registers.b;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = 1;
		cpu->registers.flag_c = 0;
		break;
	case 0xA1: /* and c */
		cpu->registers.a &= cpu->registers.c;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = 1;
		cpu->registers.flag_c = 0;
		break;
	case 0xA2: /* and d */
		cpu->registers.a &= cpu->registers.d;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = 1;
		cpu->registers.flag_c = 0;
		break;
	case 0xA3: /* and e */
		cpu->registers.a &= cpu->registers.e;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = 1;
		cpu->registers.flag_c = 0;
		break;
	case 0xA4: /* and h */
		cpu->registers.a &= cpu->registers.h;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = 1;
		cpu->registers.flag_c = 0;
		break;
	case 0xA5: /* and l */
		cpu->registers.a &= cpu->registers.l;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = 1;
		cpu->registers.flag_c = 0;
		break;
	case 0xA6: /* and (hl) */
		cpu->registers.a &= mmu_peek8(mmu, cpu->registers.hl);
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = 1;
		cpu->registers.flag_c = 0;
		break;
	case 0xA7: /* and a */
		cpu->registers.a &= cpu->registers.a;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = 1;
		cpu->registers.flag_c = 0;
		break;
	case 0xA8: /* xor b */
		cpu->registers.a ^= cpu->registers.b;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = 0;
		cpu->registers.flag_c = 0;
		break;
	case 0xA9: /* xor c */
		cpu->registers.a ^= cpu->registers.c;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = 0;
		cpu->registers.flag_c = 0;
		break;
	case 0xAA: /* xor d */
		cpu->registers.a ^= cpu->registers.d;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = 0;
		cpu->registers.flag_c = 0;
		break;
	case 0xAB: /* xor e */
		cpu->registers.a ^= cpu->registers.e;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = 0;
		cpu->registers.flag_c = 0;
		break;
	case 0xAC: /* xor h */
		cpu->registers.a ^= cpu->registers.h;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = 0;
		cpu->registers.flag_c = 0;
		break;
	case 0xAD: /* xor l */
		cpu->registers.a ^= cpu->registers.l;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = 0;
		cpu->registers.flag_c = 0;
		break;
	case 0xAE: /* xor (hl) */
		cpu->registers.a ^= mmu_peek8(mmu, cpu->registers.hl);
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = 0;
		cpu->registers.flag_c = 0;
		break;
	case 0xAF: /* xor a */
		cpu->registers.a ^= cpu->registers.a;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = 0;
		cpu->registers.flag_c = 0;
		break;
	case 0xB0: /* or b */
		cpu->registers.a |= cpu->registers.b;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = 0;
		cpu->registers.flag_c = 0;
		break;
	case 0xB1: /* or c */
		cpu->registers.a |= cpu->registers.c;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = 0;
		cpu->registers.flag_c = 0;
		break;
	case 0xB2: /* or d */
		cpu->registers.a |= cpu->registers.d;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = 0;
		cpu->registers.flag_c = 0;
		break;
	case 0xB3: /* or e */
		cpu->registers.a |= cpu->registers.e;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = 0;
		cpu->registers.flag_c = 0;
		break;
	case 0xB4: /* or h */
		cpu->registers.a |= cpu->registers.h;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = 0;
		cpu->registers.flag_c = 0;
		break;
	case 0xB5: /* or l */
		cpu->registers.a |= cpu->registers.l;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = 0;
		cpu->registers.flag_c = 0;
		break;
	case 0xB6: /* or (hl) */
		cpu->registers.a |= mmu_peek8(mmu, cpu->registers.hl);
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = 0;
		cpu->registers.flag_c = 0;
		break;
	case 0xB7: /* or a */
		cpu->registers.a |= cpu->registers.a;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = 0;
		cpu->registers.flag_c = 0;
		break;
	case 0xB8: /* cp b */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0xB9: /* cp c */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0xBA: /* cp d */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0xBB: /* cp e */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0xBC: /* cp h */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0xBD: /* cp l */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0xBE: /* cp (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0xBF: /* cp a */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0xC0: /* ret nz */
		if (!cpu->registers.flag_z)
		{
			cpu_ret(cpu, mmu);
			cpu->clock.cycles += 12;
		}
		break;
	case 0xC1: /* pop bc */
		cpu->registers.bc = cpu_pop(cpu, mmu);
		break;
	case 0xC2: /* jp nz, a16 */
		if (!cpu->registers.flag_z)
		{
			cpu->registers.pc = imm16;
			cpu->clock.cycles += 4;
		}
		break;
	case 0xC3: /* jp a16 */
		cpu->registers.pc = imm16;
		break;
	case 0xC4: /* call nz, a16 */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0xC5: /* push bc */
		cpu_push(cpu, mmu, cpu->registers.bc);
		break;
	case 0xC6: /* add a, d8 */
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = HALF_CARRY(cpu->registers.a, imm8);
		cpu->registers.flag_h = FULL_CARRY(cpu->registers.a, imm8);
		cpu->registers.a += imm8;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		break;
	case 0xC7: /* rst 00h */
		cpu_call(cpu, mmu, 0x00);
		break;
	case 0xC8: /* ret z */
		if (cpu->registers.flag_z)
		{
			cpu_ret(cpu, mmu);
			cpu->clock.cycles += 12;
		}
		break;
	case 0xC9: /* ret */
		cpu_ret(cpu, mmu);
		break;
	case 0xCA: /* jp z, a16 */
		if (cpu->registers.flag_z)
		{
			cpu->registers.pc = imm16;
			cpu->clock.cycles += 4;
		}
		break;
	case 0xCB: /* prefix cb */
		cpu_execute_cb(cpu, mmu, imm8);
		break;
	case 0xCC: /* call z, a16 */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0xCD: /* call a16 */
		cpu_call(cpu, mmu, imm16);
		break;
	case 0xCE: /* adc a, d8 */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0xCF: /* rst 08h */
		cpu_call(cpu, mmu, 0x08);
		break;
	case 0xD0: /* ret nc */
		if (!cpu->registers.flag_c)
		{
			cpu_ret(cpu, mmu);
			cpu->clock.cycles += 12;
		}
		break;
	case 0xD1: /* pop de */
		cpu->registers.de = cpu_pop(cpu, mmu);
		break;
	case 0xD2: /* jp nc, a16 */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0xD4: /* call nc, a16 */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0xD5: /* push de */
		cpu_push(cpu, mmu, cpu->registers.de);
		break;
	case 0xD6: /* sub d8 */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0xD7: /* rst 10h */
		cpu_call(cpu, mmu, 0x10);
		break;
	case 0xD8: /* ret c */
		if (cpu->registers.flag_c)
		{
			cpu_ret(cpu, mmu);
			cpu->clock.cycles += 12;
		}
		break;
	case 0xD9: /* reti */
		cpu_ret(cpu, mmu);
		cpu->interrupt.master = true;
		cpu->interrupt.delay = 1;
		break;
	case 0xDA: /* jp c, a16 */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0xDC: /* call c, a16 */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0xDE: /* sbc a, d8 */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0xDF: /* rst 18h */
		cpu_call(cpu, mmu, 0x18);
		break;
	case 0xE0: /* ldh (a8), a */
		mmu_poke8(mmu, 0xFF00 + imm8, cpu->registers.a);
		break;
	case 0xE1: /* pop hl */
		cpu->registers.hl = cpu_pop(cpu, mmu);
		break;
	case 0xE2: /* ld (c), a */
		mmu_poke8(mmu, 0xFF00 + cpu->registers.c, cpu->registers.a);
		break;
	case 0xE5: /* push hl */
		cpu_push(cpu, mmu, cpu->registers.hl);
		break;
	case 0xE6: /* and d8 */
		cpu->registers.a &= imm8;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		cpu->registers.flag_n = 0;
		cpu->registers.flag_h = 1;
		cpu->registers.flag_c = 0;
		break;
	case 0xE7: /* rst 20h */
		cpu_call(cpu, mmu, 0x20);
		break;
	case 0xE8: /* add sp, r8 */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0xE9: /* jp hl */
		cpu->registers.pc = cpu->registers.hl;
		break;
	case 0xEA: /* ld (a16), a */
		mmu_poke8(mmu, imm16, cpu->registers.a);
		break;
	case 0xEE: /* xor d8 */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0xEF: /* rst 28h */
		cpu_call(cpu, mmu, 0x28);
		break;
	case 0xF0: /* ldh a, (a8) */
		cpu->registers.a = mmu_peek8(mmu, 0xFF00 + imm8);
		break;
	case 0xF1: /* pop af */
		cpu->registers.af = cpu_pop(cpu, mmu);
		break;
	case 0xF2: /* ld a, (c) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0xF3: /* di */
		cpu->interrupt.master = false;
		break;
	case 0xF5: /* push af */
		cpu_push(cpu, mmu, cpu->registers.af);
		break;
	case 0xF6: /* or d8 */
		cpu->registers.a |= imm8;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = false;
		cpu->registers.flag_c = false;
		break;
	case 0xF7: /* rst 30h */
		cpu_call(cpu, mmu, 0x30);
		break;
	case 0xF8: /* ld hl, sp+r8 */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0xF9: /* ld sp, hl */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented opcode");
		break;
	case 0xFA: /* ld a, (a16) */
		cpu->registers.a = mmu_peek8(mmu, imm16);
		break;
	case 0xFB: /* ei */
		cpu->interrupt.master = true;
		break;
	case 0xFE: /* cp d8 */
		cpu->registers.flag_z = cpu->registers.a == imm8;
		cpu->registers.flag_n = 1;
		cpu->registers.flag_h = (cpu->registers.a & 0xF) < (imm8 & 0xF);
		cpu->registers.flag_c = cpu->registers.a < imm8;
		break;
	case 0xFF: /* rst 38h */
		cpu_call(cpu, mmu, 0x38);
		break;
	default:
		cpu_fault(cpu, mmu, opc, "undefined opcode");
		break;
	}
}

void cpu_execute_cb(cpu_t* cpu, mmu_t* mmu, uint8_t opcode)
{
	/* decode opcode & immediate values */
	opc_t* opc = &opc_opcodes_cb[opcode];

	/* update state */
	cpu->registers.pc += opc->length - 1; // subtract one because length of prefix is already counted
	cpu->clock.cycles = opc->cycles;

	switch (opcode)
	{
	case 0x00: /* rlc b */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x01: /* rlc c */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x02: /* rlc d */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x03: /* rlc e */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x04: /* rlc h */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x05: /* rlc l */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x06: /* rlc (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x07: /* rlc a */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x08: /* rrc b */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x09: /* rrc c */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x0A: /* rrc d */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x0B: /* rrc e */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x0C: /* rrc h */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x0D: /* rrc l */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x0E: /* rrc (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x0F: /* rrc a */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x10: /* rl b */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x11: /* rl c */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x12: /* rl d */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x13: /* rl e */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x14: /* rl h */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x15: /* rl l */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x16: /* rl (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x17: /* rl a */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x18: /* rr b */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x19: /* rr c */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x1A: /* rr d */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x1B: /* rr e */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x1C: /* rr h */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x1D: /* rr l */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x1E: /* rr (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x1F: /* rr a */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x20: /* sla b */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x21: /* sla c */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x22: /* sla d */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x23: /* sla e */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x24: /* sla h */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x25: /* sla l */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x26: /* sla (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x27: /* sla a */
		cpu->registers.flag_c = cpu->registers.a >> 7;
		cpu->registers.a = cpu->registers.a << 1;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = false;
		break;
	case 0x28: /* sra b */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x29: /* sra c */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x2A: /* sra d */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x2B: /* sra e */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x2C: /* sra h */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x2D: /* sra l */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x2E: /* sra (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x2F: /* sra a */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x30: /* swap b */
		cpu->registers.b = (cpu->registers.b << 4) | (cpu->registers.b >> 4);
		cpu->registers.flag_z = IS_ZERO(cpu->registers.b);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = false;
		cpu->registers.flag_c = false;
		break;
	case 0x31: /* swap c */
		cpu->registers.c = (cpu->registers.c << 4) | (cpu->registers.c >> 4);
		cpu->registers.flag_z = IS_ZERO(cpu->registers.c);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = false;
		cpu->registers.flag_c = false;
		break;
	case 0x32: /* swap d */
		cpu->registers.d = (cpu->registers.d << 4) | (cpu->registers.d >> 4);
		cpu->registers.flag_z = IS_ZERO(cpu->registers.d);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = false;
		cpu->registers.flag_c = false;
		break;
	case 0x33: /* swap e */
		cpu->registers.e = (cpu->registers.e << 4) | (cpu->registers.e >> 4);
		cpu->registers.flag_z = IS_ZERO(cpu->registers.e);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = false;
		cpu->registers.flag_c = false;
		break;
	case 0x34: /* swap h */
		cpu->registers.h = (cpu->registers.h << 4) | (cpu->registers.h >> 4);
		cpu->registers.flag_z = IS_ZERO(cpu->registers.h);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = false;
		cpu->registers.flag_c = false;
		break;
	case 0x35: /* swap l */
		cpu->registers.l = (cpu->registers.l << 4) | (cpu->registers.l >> 4);
		cpu->registers.flag_z = IS_ZERO(cpu->registers.l);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = false;
		cpu->registers.flag_c = false;
		break;
	case 0x36: /* swap (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x37: /* swap a */
		cpu->registers.a = (cpu->registers.a << 4) | (cpu->registers.a >> 4);
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = false;
		cpu->registers.flag_c = false;
		break;
	case 0x38: /* srl b */
		cpu->registers.flag_c = cpu->registers.b & 0x1;
		cpu->registers.b = cpu->registers.b >> 1;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.b);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = false;
		break;
	case 0x39: /* srl c */
		cpu->registers.flag_c = cpu->registers.c & 0x1;
		cpu->registers.c = cpu->registers.c >> 1;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.c);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = false;
		break;
	case 0x3A: /* srl d */
		cpu->registers.flag_c = cpu->registers.d & 0x1;
		cpu->registers.d = cpu->registers.d >> 1;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.d);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = false;
		break;
	case 0x3B: /* srl e */
		cpu->registers.flag_c = cpu->registers.e & 0x1;
		cpu->registers.e = cpu->registers.e >> 1;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.e);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = false;
		break;
	case 0x3C: /* srl h */
		cpu->registers.flag_c = cpu->registers.h & 0x1;
		cpu->registers.h = cpu->registers.h >> 1;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.h);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = false;
		break;
	case 0x3D: /* srl l */
		cpu->registers.flag_c = cpu->registers.l & 0x1;
		cpu->registers.l = cpu->registers.l >> 1;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.l);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = false;
		break;
	case 0x3E: /* srl (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x3F: /* srl a */
		cpu->registers.flag_c = cpu->registers.a & 0x1;
		cpu->registers.a = cpu->registers.a >> 1;
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = false;
		break;
	case 0x40: /* bit 0, b */
		cpu->registers.flag_z = !(cpu->registers.b & 0x01);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x41: /* bit 0, c */
		cpu->registers.flag_z = !(cpu->registers.c & 0x01);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x42: /* bit 0, d */
		cpu->registers.flag_z = !(cpu->registers.d & 0x01);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x43: /* bit 0, e */
		cpu->registers.flag_z = !(cpu->registers.e & 0x01);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x44: /* bit 0, h */
		cpu->registers.flag_z = !(cpu->registers.h & 0x01);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x45: /* bit 0, l */
		cpu->registers.flag_z = !(cpu->registers.l & 0x01);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x46: /* bit 0, (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x47: /* bit 0, a */
		cpu->registers.flag_z = !(cpu->registers.a & 0x01);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x48: /* bit 1, b */
		cpu->registers.flag_z = !(cpu->registers.b & 0x02);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x49: /* bit 1, c */
		cpu->registers.flag_z = !(cpu->registers.c & 0x2);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x4A: /* bit 1, d */
		cpu->registers.flag_z = !(cpu->registers.d & 0x02);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x4B: /* bit 1, e */
		cpu->registers.flag_z = !(cpu->registers.e & 0x02);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x4C: /* bit 1, h */
		cpu->registers.flag_z = !(cpu->registers.h & 0x02);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x4D: /* bit 1, l */
		cpu->registers.flag_z = !(cpu->registers.l & 0x02);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x4E: /* bit 1, (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x4F: /* bit 1, a */
		cpu->registers.flag_z = !(cpu->registers.a & 0x02);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x50: /* bit 2, b */
		cpu->registers.flag_z = !(cpu->registers.b & 0x04);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x51: /* bit 2, c */
		cpu->registers.flag_z = !(cpu->registers.c & 0x04);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x52: /* bit 2, d */
		cpu->registers.flag_z = !(cpu->registers.d & 0x04);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x53: /* bit 2, e */
		cpu->registers.flag_z = !(cpu->registers.e & 0x04);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x54: /* bit 2, h */
		cpu->registers.flag_z = !(cpu->registers.h & 0x04);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x55: /* bit 2, l */
		cpu->registers.flag_z = !(cpu->registers.l & 0x04);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x56: /* bit 2, (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x57: /* bit 2, a */
		cpu->registers.flag_z = !(cpu->registers.a & 0x04);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x58: /* bit 3, b */
		cpu->registers.flag_z = !(cpu->registers.b & 0x08);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x59: /* bit 3, c */
		cpu->registers.flag_z = !(cpu->registers.c & 0x08);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x5A: /* bit 3, d */
		cpu->registers.flag_z = !(cpu->registers.d & 0x08);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x5B: /* bit 3, e */
		cpu->registers.flag_z = !(cpu->registers.e & 0x08);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x5C: /* bit 3, h */
		cpu->registers.flag_z = !(cpu->registers.h & 0x08);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x5D: /* bit 3, l */
		cpu->registers.flag_z = !(cpu->registers.l & 0x08);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x5E: /* bit 3, (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x5F: /* bit 3, a */
		cpu->registers.flag_z = !(cpu->registers.a & 0x08);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x60: /* bit 4, b */
		cpu->registers.flag_z = !(cpu->registers.b & 0x10);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x61: /* bit 4, c */
		cpu->registers.flag_z = !(cpu->registers.c & 0x10);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x62: /* bit 4, d */
		cpu->registers.flag_z = !(cpu->registers.d & 0x10);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x63: /* bit 4, e */
		cpu->registers.flag_z = !(cpu->registers.e & 0x10);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x64: /* bit 4, h */
		cpu->registers.flag_z = !(cpu->registers.h & 0x10);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x65: /* bit 4, l */
		cpu->registers.flag_z = !(cpu->registers.l & 0x10);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x66: /* bit 4, (hl) */
		cpu->registers.flag_z = !(cpu->registers.c & 0x10);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x67: /* bit 4, a */
		cpu->registers.flag_z = !(cpu->registers.a & 0x10);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x68: /* bit 5, b */
		cpu->registers.flag_z = !(cpu->registers.b & 0x20);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x69: /* bit 5, c */
		cpu->registers.flag_z = !(cpu->registers.c & 0x20);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x6A: /* bit 5, d */
		cpu->registers.flag_z = !(cpu->registers.d & 0x20);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x6B: /* bit 5, e */
		cpu->registers.flag_z = !(cpu->registers.e & 0x20);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x6C: /* bit 5, h */
		cpu->registers.flag_z = !(cpu->registers.h & 0x20);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x6D: /* bit 5, l */
		cpu->registers.flag_z = !(cpu->registers.l & 0x20);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x6E: /* bit 5, (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x6F: /* bit 5, a */
		cpu->registers.flag_z = !(cpu->registers.a & 0x20);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x70: /* bit 6, b */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x71: /* bit 6, c */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x72: /* bit 6, d */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x73: /* bit 6, e */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x74: /* bit 6, h */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x75: /* bit 6, l */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x76: /* bit 6, (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x77: /* bit 6, a */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x78: /* bit 7, b */
		cpu->registers.flag_z = IS_ZERO(cpu->registers.b & 0x80);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x79: /* bit 7, c */
		cpu->registers.flag_z = IS_ZERO(cpu->registers.c & 0x80);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x7A: /* bit 7, d */
		cpu->registers.flag_z = IS_ZERO(cpu->registers.d & 0x80);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x7B: /* bit 7, e */
		cpu->registers.flag_z = IS_ZERO(cpu->registers.e & 0x80);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x7C: /* bit 7, h */
		cpu->registers.flag_z = IS_ZERO(cpu->registers.h & 0x80);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x7D: /* bit 7, l */
		cpu->registers.flag_z = IS_ZERO(cpu->registers.l & 0x80);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x7E: /* bit 7, (hl) */
		cpu->registers.flag_z = IS_ZERO(mmu_peek8(mmu, cpu->registers.hl) & 0x80);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x7F: /* bit 7, a */
		cpu->registers.flag_z = IS_ZERO(cpu->registers.a & 0x80);
		cpu->registers.flag_n = false;
		cpu->registers.flag_h = true;
		break;
	case 0x80: /* res 0, b */
		cpu->registers.b &= 0xFE;
		break;
	case 0x81: /* res 0, c */
		cpu->registers.c &= 0xFE;
		break;
	case 0x82: /* res 0, d */
		cpu->registers.d &= 0xFE;
		break;
	case 0x83: /* res 0, e */
		cpu->registers.e &= 0xFE;
		break;
	case 0x84: /* res 0, h */
		cpu->registers.h &= 0xFE;
		break;
	case 0x85: /* res 0, l */
		cpu->registers.l &= 0xFE;
		break;
	case 0x86: /* res 0, (hl) */
		mmu_poke8(mmu, cpu->registers.hl, mmu_peek8(mmu, cpu->registers.hl) & 0xFE);
		break;
	case 0x87: /* res 0, a */
		cpu->registers.a &= 0xFE;
		break;
	case 0x88: /* res 1, b */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x89: /* res 1, c */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x8A: /* res 1, d */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x8B: /* res 1, e */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x8C: /* res 1, h */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x8D: /* res 1, l */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x8E: /* res 1, (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x8F: /* res 1, a */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x90: /* res 2, b */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x91: /* res 2, c */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x92: /* res 2, d */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x93: /* res 2, e */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x94: /* res 2, h */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x95: /* res 2, l */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x96: /* res 2, (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x97: /* res 2, a */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x98: /* res 3, b */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x99: /* res 3, c */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x9A: /* res 3, d */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x9B: /* res 3, e */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x9C: /* res 3, h */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x9D: /* res 3, l */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x9E: /* res 3, (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0x9F: /* res 3, a */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xA0: /* res 4, b */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xA1: /* res 4, c */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xA2: /* res 4, d */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xA3: /* res 4, e */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xA4: /* res 4, h */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xA5: /* res 4, l */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xA6: /* res 4, (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xA7: /* res 4, a */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xA8: /* res 5, b */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xA9: /* res 5, c */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xAA: /* res 5, d */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xAB: /* res 5, e */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xAC: /* res 5, h */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xAD: /* res 5, l */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xAE: /* res 5, (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xAF: /* res 5, a */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xB0: /* res 6, b */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xB1: /* res 6, c */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xB2: /* res 6, d */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xB3: /* res 6, e */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xB4: /* res 6, h */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xB5: /* res 6, l */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xB6: /* res 6, (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xB7: /* res 6, a */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xB8: /* res 7, b */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xB9: /* res 7, c */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xBA: /* res 7, d */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xBB: /* res 7, e */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xBC: /* res 7, h */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xBD: /* res 7, l */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xBE: /* res 7, (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xBF: /* res 7, a */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xC0: /* set 0, b */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xC1: /* set 0, c */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xC2: /* set 0, d */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xC3: /* set 0, e */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xC4: /* set 0, h */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xC5: /* set 0, l */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xC6: /* set 0, (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xC7: /* set 0, a */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xC8: /* set 1, b */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xC9: /* set 1, c */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xCA: /* set 1, d */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xCB: /* set 1, e */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xCC: /* set 1, h */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xCD: /* set 1, l */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xCE: /* set 1, (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xCF: /* set 1, a */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xD0: /* set 2, b */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xD1: /* set 2, c */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xD2: /* set 2, d */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xD3: /* set 2, e */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xD4: /* set 2, h */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xD5: /* set 2, l */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xD6: /* set 2, (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xD7: /* set 2, a */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xD8: /* set 3, b */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xD9: /* set 3, c */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xDA: /* set 3, d */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xDB: /* set 3, e */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xDC: /* set 3, h */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xDD: /* set 3, l */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xDE: /* set 3, (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xDF: /* set 3, a */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xE0: /* set 4, b */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xE1: /* set 4, c */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xE2: /* set 4, d */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xE3: /* set 4, e */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xE4: /* set 4, h */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xE5: /* set 4, l */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xE6: /* set 4, (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xE7: /* set 4, a */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xE8: /* set 5, b */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xE9: /* set 5, c */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xEA: /* set 5, d */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xEB: /* set 5, e */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xEC: /* set 5, h */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xED: /* set 5, l */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xEE: /* set 5, (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xEF: /* set 5, a */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xF0: /* set 6, b */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xF1: /* set 6, c */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xF2: /* set 6, d */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xF3: /* set 6, e */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xF4: /* set 6, h */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xF5: /* set 6, l */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xF6: /* set 6, (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xF7: /* set 6, a */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xF8: /* set 7, b */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xF9: /* set 7, c */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xFA: /* set 7, d */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xFB: /* set 7, e */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xFC: /* set 7, h */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xFD: /* set 7, l */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xFE: /* set 7, (hl) */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	case 0xFF: /* set 7, a */
		// OPERATION
		// FLAGS
		cpu_fault(cpu, mmu, opc, "unimplemented cb opcode");
		break;
	default:
		cpu_fault(cpu, mmu, opc, "undefined cb opcode");
		break;
	}
}

void cpu_request(cpu_t* cpu, mmu_t* mmu, uint8_t index)
{
	/* set interrupt request flag bit */
	mmu->io[IRF & 0xFF] |= index;
}

void cpu_interrupt(cpu_t* cpu, mmu_t* mmu, uint16_t address)
{
	/* disable interrupts */
	cpu->interrupt.master = false;

	/* call interrupt-vector */
	cpu_call(cpu, mmu, address);
}

void cpu_cycle(cpu_t* cpu, mmu_t* mmu)
{
	/* fetch opcode */
	uint8_t opcode = mmu_peek8(mmu, cpu->registers.pc);

	/* execute */
	// cpu_trace(cpu, opc);
	cpu_execute(cpu, mmu, opcode);
	// cpu_dump(cpu);

	/* execute interrupts */
	if (cpu->interrupt.master && !cpu->interrupt.delay)
	{
		uint8_t interrupt_enable = mmu->interrupt_enable;
		uint8_t interrupt_request = mmu->io[IRF & 0xFF];
		uint8_t interrupt_flags = interrupt_enable & interrupt_request;

		if (interrupt_flags & INT_V_BLANK_INDEX)
		{
			/* v-blank interrupt */
			cpu_interrupt(cpu, mmu, INT_V_BLANK);
		}
		else if (interrupt_flags & INT_LCD_STAT_INDEX)
		{
			/* lcd-stat interrupt */
			cpu_interrupt(cpu, mmu, INT_LCD_STAT);
		}
		else if (interrupt_flags & INT_TIMER_INDEX)
		{
			/* timer interrupt */
			cpu_interrupt(cpu, mmu, INT_TIMER);
		}
		else if (interrupt_flags & INT_SERIAL_INDEX)
		{
			/* serial interrupt */
			cpu_interrupt(cpu, mmu, INT_SERIAL);
		}
		else if (interrupt_flags & INT_JOYPAD_INDEX)
		{
			/* joypad interrupt */
			cpu_interrupt(cpu, mmu, INT_JOYPAD);
		}
	}
	else if (cpu->interrupt.delay)
	{
		cpu->interrupt.delay--;
	}

	/* timer interrupt */
	cpu->clock.div_diff += cpu->clock.cycles;
	if (cpu->clock.div_diff >= 16)
	{
		mmu->hram[INT_TIMER_DIV & 0xFF]++;
		cpu->clock.div_diff -= 16;

		if (mmu->hram[INT_TIMER_TAC & 0xFF] & 0x4)
		{
			uint8_t og_tima = mmu->hram[INT_TIMER_TIMA & 0xFF];
			switch (mmu->hram[INT_TIMER_TAC & 0xFF] & 0x3)
			{
			case 0b00:
				if (cpu->clock.tima_mod % 64) mmu->hram[INT_TIMER_TIMA & 0xFF]++;
				break;
			case 0b01:
				if (cpu->clock.tima_mod % 1) mmu->hram[INT_TIMER_TIMA & 0xFF]++;
				break;
			case 0b10:
				if (cpu->clock.tima_mod % 4) mmu->hram[INT_TIMER_TIMA & 0xFF]++;
				break;
			case 0b11:
				if (cpu->clock.tima_mod % 16) mmu->hram[INT_TIMER_TIMA & 0xFF]++;
				break;
			}

			if (mmu->hram[INT_TIMER_TIMA & 0xFF] < og_tima)
			{
				cpu_request(cpu, mmu, INT_TIMER_INDEX);
				mmu->hram[INT_TIMER_TIMA & 0xFF] = mmu->hram[INT_TIMER_TMA & 0xFF];
			}
		}

		cpu->clock.tima_mod++;
	}
}
