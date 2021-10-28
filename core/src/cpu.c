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

	/* setup interrupts */
	cpu->interrupt.delay = 0;
	cpu->interrupt.master = true;
}

void cpu_destroy(cpu_t* cpu)
{
	
}

void cpu_fault(cpu_t* cpu, opc_t* opc, const char* message)
{
	cpu_trace(cpu, opc);
	printf("            ^~~~ fault received `%s`\n", message);
	printf("[!] encountered fatal cpu exception\n");
	printf("    -- core dump --\n");
	cpu_dump(cpu);
	exit(EXIT_FAILURE);
}

void cpu_trace(cpu_t* cpu, opc_t* opc)
{
	printf("[+] 0x%lX: %s\n", cpu->registers.pc - opc->length, opc->disasm);
}

void cpu_dump(cpu_t* cpu)
{
	printf("    af: %04X\n", cpu->registers.af);
	printf("    bc: %04X\n", cpu->registers.bc);
	printf("    de: %04X\n", cpu->registers.de);
	printf("    hl: %04X\n", cpu->registers.hl);
	printf("    sp: %04X\n", cpu->registers.sp);
	printf("    pc: %04X\n", cpu->registers.pc);
	printf("   ime: %s\n", cpu->interrupt.master ? "enabled" : "disabled");
}

void cpu_call(cpu_t* cpu, mmu_t* mmu, uint16_t address)
{
	/* write address */
	mmu_poke16(mmu, cpu->registers.sp, cpu->registers.pc);

	/* decrement stack */
	cpu->registers.sp -= sizeof(uint16_t);
}

void cpu_ret(cpu_t* cpu, mmu_t* mmu)
{
	/* jump back */
	cpu->registers.pc = mmu_peek16(mmu, cpu->registers.sp);

	/* increment stack */
	cpu->registers.sp += sizeof(uint16_t);
}

void cpu_execute(cpu_t* cpu, mmu_t* mmu, uint8_t opcode)
{
	/* decode opcode & immediate values */
	opc_t* opc = &opc_opcodes[opcode];
	uint8_t imm8 = mmu_peek8(mmu, cpu->registers.pc + 1);
	uint16_t imm16 = mmu_peek16(mmu, cpu->registers.pc + 1);

	/* update state */
	cpu->registers.pc += opc->length;
	cpu->clock.cycles += opc->cycles;

	/* execute based on opcode */
	switch (opcode)
	{
	case 0x00: /* nop */
	    break;
	case 0x01: /* ld bc, d16 */
	    cpu->registers.bc = imm8;
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
		cpu_fault(cpu, opc, "unimplemented opcode");
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
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x10: /* stop 0 */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x11: /* ld de, d16 */
	    cpu->registers.de = imm16;
	    break;
	case 0x12: /* ld (de), a */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
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
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x18: /* jr r8 */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x19: /* add hl, de */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x1A: /* ld a, (de) */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
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
		cpu_fault(cpu, opc, "unimplemented opcode");
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
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
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
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x28: /* jr z, r8 */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x29: /* add hl, hl */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x2A: /* ld a, (hl+) */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
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
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x30: /* jr nc, r8 */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
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
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x35: /* dec (hl) */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x36: /* ld (hl), d8 */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x37: /* scf */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x38: /* jr c, r8 */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x39: /* add hl, sp */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x3A: /* ld a, (hl-) */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
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
		cpu_fault(cpu, opc, "unimplemented opcode");
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
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
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
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
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
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
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
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
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
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
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
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x6F: /* ld l, a */
	    cpu->registers.l = cpu->registers.a;
	    break;
	case 0x70: /* ld (hl), b */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x71: /* ld (hl), c */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x72: /* ld (hl), d */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x73: /* ld (hl), e */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x74: /* ld (hl), h */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x75: /* ld (hl), l */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x76: /* halt */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x77: /* ld (hl), a */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x78: /* ld a, b */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x79: /* ld a, c */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x7A: /* ld a, d */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x7B: /* ld a, e */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x7C: /* ld a, h */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x7D: /* ld a, l */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x7E: /* ld a, (hl) */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x7F: /* ld a, a */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x80: /* add a, b */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x81: /* add a, c */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x82: /* add a, d */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x83: /* add a, e */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x84: /* add a, h */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x85: /* add a, l */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x86: /* add a, (hl) */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x87: /* add a, a */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x88: /* adc a, b */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x89: /* adc a, c */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x8A: /* adc a, d */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x8B: /* adc a, e */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x8C: /* adc a, h */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x8D: /* adc a, l */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x8E: /* adc a, (hl) */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x8F: /* adc a, a */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x90: /* sub b */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x91: /* sub c */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x92: /* sub d */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x93: /* sub e */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x94: /* sub h */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x95: /* sub l */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x96: /* sub (hl) */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x97: /* sub a */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x98: /* sbc a, b */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x99: /* sbc a, c */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x9A: /* sbc a, d */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x9B: /* sbc a, e */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x9C: /* sbc a, h */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x9D: /* sbc a, l */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x9E: /* sbc a, (hl) */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0x9F: /* sbc a, a */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xA0: /* and b */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xA1: /* and c */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xA2: /* and d */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xA3: /* and e */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xA4: /* and h */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xA5: /* and l */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xA6: /* and (hl) */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xA7: /* and a */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xA8: /* xor b */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xA9: /* xor c */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xAA: /* xor d */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xAB: /* xor e */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xAC: /* xor h */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xAD: /* xor l */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xAE: /* xor (hl) */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xAF: /* xor a */
	    cpu->registers.a ^= cpu->registers.a;

	    cpu->registers.flag_z = IS_ZERO(cpu->registers.a);
	    cpu->registers.flag_n = 0;
	    cpu->registers.flag_h = 0;
	    cpu->registers.flag_c = 0;
	    break;
	case 0xB0: /* or b */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xB1: /* or c */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xB2: /* or d */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xB3: /* or e */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xB4: /* or h */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xB5: /* or l */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xB6: /* or (hl) */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xB7: /* or a */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xB8: /* cp b */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xB9: /* cp c */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xBA: /* cp d */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xBB: /* cp e */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xBC: /* cp h */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xBD: /* cp l */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xBE: /* cp (hl) */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xBF: /* cp a */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xC0: /* ret nz */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xC1: /* pop bc */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xC2: /* jp nz, a16 */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xC3: /* jp a16 */
	    cpu->registers.pc = imm16;
	    break;
	case 0xC4: /* call nz, a16 */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xC5: /* push bc */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xC6: /* add a, d8 */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xC7: /* rst 00h */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xC8: /* ret z */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xC9: /* ret */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xCA: /* jp z, a16 */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xCB: /* prefix cb */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xCC: /* call z, a16 */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xCD: /* call a16 */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xCE: /* adc a, d8 */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xCF: /* rst 08h */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xD0: /* ret nc */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xD1: /* pop de */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xD2: /* jp nc, a16 */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xD4: /* call nc, a16 */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xD5: /* push de */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xD6: /* sub d8 */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xD7: /* rst 10h */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xD8: /* ret c */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xD9: /* reti */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xDA: /* jp c, a16 */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xDC: /* call c, a16 */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xDE: /* sbc a, d8 */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xDF: /* rst 18h */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xE0: /* ldh (a8), a */
	    mmu_poke8(mmu, 0xFF00 + imm8, cpu->registers.a);
	    break;
	case 0xE1: /* pop hl */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xE2: /* ld (c), a */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xE5: /* push hl */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xE6: /* and d8 */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xE7: /* rst 20h */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xE8: /* add sp, r8 */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xE9: /* jp (hl) */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xEA: /* ld (a16), a */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xEE: /* xor d8 */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xEF: /* rst 28h */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xF0: /* ldh a, (a8) */
	    cpu->registers.a = mmu_peek8(mmu, 0xFF00 + imm8);
	    break;
	case 0xF1: /* pop af */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xF2: /* ld a, (c) */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xF3: /* di */
	    cpu->interrupt.master = false;
	    break;
	case 0xF5: /* push af */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xF6: /* or d8 */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xF7: /* rst 30h */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xF8: /* ld hl, sp+r8 */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xF9: /* ld sp, hl */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	case 0xFA: /* ld a, (a16) */
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
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
	    // OPERATION
	    // FLAGS
		cpu_fault(cpu, opc, "unimplemented opcode");
	    break;
	default:
		cpu_fault(cpu, opc, "undefined opcode");
	    break;
	}
}

void cpu_request(cpu_t* cpu, mmu_t* mmu, uint8_t index)
{
	/* set interrupt request flag bit */
	mmu->io[IRF & 0xFF] |= (1 << index);
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

	/* request interrupts */
	cpu_request(cpu, mmu, INT_V_BLANK_INDEX);

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
}
