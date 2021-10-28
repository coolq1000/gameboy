#include "core/opc.h"

#include "core/cpu.h"

opc_t opc_opcodes[256] = {
	/* 0x00 */ { "nop", 1, 4 },
	/* 0x01 */ { "ld bc, d16", 3, 12 },
	/* 0x02 */ { "ld (bc), a", 1, 8 },
	/* 0x03 */ { "inc bc", 1, 8 },
	/* 0x04 */ { "inc b", 1, 4 },
	/* 0x05 */ { "dec b", 1, 4 },
	/* 0x06 */ { "ld b, d8", 2, 8 },
	/* 0x07 */ { "rlca", 1, 4 },
	/* 0x08 */ { "ld (a16), sp", 3, 20 },
	/* 0x09 */ { "add hl, bc", 1, 8 },
	/* 0x0A */ { "ld a, (bc)", 1, 8 },
	/* 0x0B */ { "dec bc", 1, 8 },
	/* 0x0C */ { "inc c", 1, 4 },
	/* 0x0D */ { "dec c", 1, 4 },
	/* 0x0E */ { "ld c, d8", 2, 8 },
	/* 0x0F */ { "rrca", 1, 4 },
	/* 0x10 */ { "stop 0", 2, 4 },
	/* 0x11 */ { "ld de, d16", 3, 12 },
	/* 0x12 */ { "ld (de), a", 1, 8 },
	/* 0x13 */ { "inc de", 1, 8 },
	/* 0x14 */ { "inc d", 1, 4 },
	/* 0x15 */ { "dec d", 1, 4 },
	/* 0x16 */ { "ld d, d8", 2, 8 },
	/* 0x17 */ { "rla", 1, 4 },
	/* 0x18 */ { "jr r8", 2, 12 },
	/* 0x19 */ { "add hl, de", 1, 8 },
	/* 0x1A */ { "ld a, (de)", 1, 8 },
	/* 0x1B */ { "dec de", 1, 8 },
	/* 0x1C */ { "inc e", 1, 4 },
	/* 0x1D */ { "dec e", 1, 4 },
	/* 0x1E */ { "ld e, d8", 2, 8 },
	/* 0x1F */ { "rra", 1, 4 },
	/* 0x20 */ { "jr nz, r8", 2, 8 },
	/* 0x21 */ { "ld hl, d16", 3, 12 },
	/* 0x22 */ { "ld (hl+), a", 1, 8 },
	/* 0x23 */ { "inc hl", 1, 8 },
	/* 0x24 */ { "inc h", 1, 4 },
	/* 0x25 */ { "dec h", 1, 4 },
	/* 0x26 */ { "ld h, d8", 2, 8 },
	/* 0x27 */ { "daa", 1, 4 },
	/* 0x28 */ { "jr z, r8", 2, 8 },
	/* 0x29 */ { "add hl, hl", 1, 8 },
	/* 0x2A */ { "ld a, (hl+)", 1, 8 },
	/* 0x2B */ { "dec hl", 1, 8 },
	/* 0x2C */ { "inc l", 1, 4 },
	/* 0x2D */ { "dec l", 1, 4 },
	/* 0x2E */ { "ld l, d8", 2, 8 },
	/* 0x2F */ { "cpl", 1, 4 },
	/* 0x30 */ { "jr nc, r8", 2, 8 },
	/* 0x31 */ { "ld sp, d16", 3, 12 },
	/* 0x32 */ { "ld (hl-), a", 1, 8 },
	/* 0x33 */ { "inc sp", 1, 8 },
	/* 0x34 */ { "inc (hl)", 1, 12 },
	/* 0x35 */ { "dec (hl)", 1, 12 },
	/* 0x36 */ { "ld (hl), d8", 2, 12 },
	/* 0x37 */ { "scf", 1, 4 },
	/* 0x38 */ { "jr c, r8", 2, 8 },
	/* 0x39 */ { "add hl, sp", 1, 8 },
	/* 0x3A */ { "ld a, (hl-)", 1, 8 },
	/* 0x3B */ { "dec sp", 1, 8 },
	/* 0x3C */ { "inc a", 1, 4 },
	/* 0x3D */ { "dec a", 1, 4 },
	/* 0x3E */ { "ld a, d8", 2, 8 },
	/* 0x3F */ { "ccf", 1, 4 },
	/* 0x40 */ { "ld b, b", 1, 4 },
	/* 0x41 */ { "ld b, c", 1, 4 },
	/* 0x42 */ { "ld b, d", 1, 4 },
	/* 0x43 */ { "ld b, e", 1, 4 },
	/* 0x44 */ { "ld b, h", 1, 4 },
	/* 0x45 */ { "ld b, l", 1, 4 },
	/* 0x46 */ { "ld b, (hl)", 1, 8 },
	/* 0x47 */ { "ld b, a", 1, 4 },
	/* 0x48 */ { "ld c, b", 1, 4 },
	/* 0x49 */ { "ld c, c", 1, 4 },
	/* 0x4A */ { "ld c, d", 1, 4 },
	/* 0x4B */ { "ld c, e", 1, 4 },
	/* 0x4C */ { "ld c, h", 1, 4 },
	/* 0x4D */ { "ld c, l", 1, 4 },
	/* 0x4E */ { "ld c, (hl)", 1, 8 },
	/* 0x4F */ { "ld c, a", 1, 4 },
	/* 0x50 */ { "ld d, b", 1, 4 },
	/* 0x51 */ { "ld d, c", 1, 4 },
	/* 0x52 */ { "ld d, d", 1, 4 },
	/* 0x53 */ { "ld d, e", 1, 4 },
	/* 0x54 */ { "ld d, h", 1, 4 },
	/* 0x55 */ { "ld d, l", 1, 4 },
	/* 0x56 */ { "ld d, (hl)", 1, 8 },
	/* 0x57 */ { "ld d, a", 1, 4 },
	/* 0x58 */ { "ld e, b", 1, 4 },
	/* 0x59 */ { "ld e, c", 1, 4 },
	/* 0x5A */ { "ld e, d", 1, 4 },
	/* 0x5B */ { "ld e, e", 1, 4 },
	/* 0x5C */ { "ld e, h", 1, 4 },
	/* 0x5D */ { "ld e, l", 1, 4 },
	/* 0x5E */ { "ld e, (hl)", 1, 8 },
	/* 0x5F */ { "ld e, a", 1, 4 },
	/* 0x60 */ { "ld h, b", 1, 4 },
	/* 0x61 */ { "ld h, c", 1, 4 },
	/* 0x62 */ { "ld h, d", 1, 4 },
	/* 0x63 */ { "ld h, e", 1, 4 },
	/* 0x64 */ { "ld h, h", 1, 4 },
	/* 0x65 */ { "ld h, l", 1, 4 },
	/* 0x66 */ { "ld h, (hl)", 1, 8 },
	/* 0x67 */ { "ld h, a", 1, 4 },
	/* 0x68 */ { "ld l, b", 1, 4 },
	/* 0x69 */ { "ld l, c", 1, 4 },
	/* 0x6A */ { "ld l, d", 1, 4 },
	/* 0x6B */ { "ld l, e", 1, 4 },
	/* 0x6C */ { "ld l, h", 1, 4 },
	/* 0x6D */ { "ld l, l", 1, 4 },
	/* 0x6E */ { "ld l, (hl)", 1, 8 },
	/* 0x6F */ { "ld l, a", 1, 4 },
	/* 0x70 */ { "ld (hl), b", 1, 8 },
	/* 0x71 */ { "ld (hl), c", 1, 8 },
	/* 0x72 */ { "ld (hl), d", 1, 8 },
	/* 0x73 */ { "ld (hl), e", 1, 8 },
	/* 0x74 */ { "ld (hl), h", 1, 8 },
	/* 0x75 */ { "ld (hl), l", 1, 8 },
	/* 0x76 */ { "halt", 1, 4 },
	/* 0x77 */ { "ld (hl), a", 1, 8 },
	/* 0x78 */ { "ld a, b", 1, 4 },
	/* 0x79 */ { "ld a, c", 1, 4 },
	/* 0x7A */ { "ld a, d", 1, 4 },
	/* 0x7B */ { "ld a, e", 1, 4 },
	/* 0x7C */ { "ld a, h", 1, 4 },
	/* 0x7D */ { "ld a, l", 1, 4 },
	/* 0x7E */ { "ld a, (hl)", 1, 8 },
	/* 0x7F */ { "ld a, a", 1, 4 },
	/* 0x80 */ { "add a, b", 1, 4 },
	/* 0x81 */ { "add a, c", 1, 4 },
	/* 0x82 */ { "add a, d", 1, 4 },
	/* 0x83 */ { "add a, e", 1, 4 },
	/* 0x84 */ { "add a, h", 1, 4 },
	/* 0x85 */ { "add a, l", 1, 4 },
	/* 0x86 */ { "add a, (hl)", 1, 8 },
	/* 0x87 */ { "add a, a", 1, 4 },
	/* 0x88 */ { "adc a, b", 1, 4 },
	/* 0x89 */ { "adc a, c", 1, 4 },
	/* 0x8A */ { "adc a, d", 1, 4 },
	/* 0x8B */ { "adc a, e", 1, 4 },
	/* 0x8C */ { "adc a, h", 1, 4 },
	/* 0x8D */ { "adc a, l", 1, 4 },
	/* 0x8E */ { "adc a, (hl)", 1, 8 },
	/* 0x8F */ { "adc a, a", 1, 4 },
	/* 0x90 */ { "sub b", 1, 4 },
	/* 0x91 */ { "sub c", 1, 4 },
	/* 0x92 */ { "sub d", 1, 4 },
	/* 0x93 */ { "sub e", 1, 4 },
	/* 0x94 */ { "sub h", 1, 4 },
	/* 0x95 */ { "sub l", 1, 4 },
	/* 0x96 */ { "sub (hl)", 1, 8 },
	/* 0x97 */ { "sub a", 1, 4 },
	/* 0x98 */ { "sbc a, b", 1, 4 },
	/* 0x99 */ { "sbc a, c", 1, 4 },
	/* 0x9A */ { "sbc a, d", 1, 4 },
	/* 0x9B */ { "sbc a, e", 1, 4 },
	/* 0x9C */ { "sbc a, h", 1, 4 },
	/* 0x9D */ { "sbc a, l", 1, 4 },
	/* 0x9E */ { "sbc a, (hl)", 1, 8 },
	/* 0x9F */ { "sbc a, a", 1, 4 },
	/* 0xA0 */ { "and b", 1, 4 },
	/* 0xA1 */ { "and c", 1, 4 },
	/* 0xA2 */ { "and d", 1, 4 },
	/* 0xA3 */ { "and e", 1, 4 },
	/* 0xA4 */ { "and h", 1, 4 },
	/* 0xA5 */ { "and l", 1, 4 },
	/* 0xA6 */ { "and (hl)", 1, 8 },
	/* 0xA7 */ { "and a", 1, 4 },
	/* 0xA8 */ { "xor b", 1, 4 },
	/* 0xA9 */ { "xor c", 1, 4 },
	/* 0xAA */ { "xor d", 1, 4 },
	/* 0xAB */ { "xor e", 1, 4 },
	/* 0xAC */ { "xor h", 1, 4 },
	/* 0xAD */ { "xor l", 1, 4 },
	/* 0xAE */ { "xor (hl)", 1, 8 },
	/* 0xAF */ { "xor a", 1, 4 },
	/* 0xB0 */ { "or b", 1, 4 },
	/* 0xB1 */ { "or c", 1, 4 },
	/* 0xB2 */ { "or d", 1, 4 },
	/* 0xB3 */ { "or e", 1, 4 },
	/* 0xB4 */ { "or h", 1, 4 },
	/* 0xB5 */ { "or l", 1, 4 },
	/* 0xB6 */ { "or (hl)", 1, 8 },
	/* 0xB7 */ { "or a", 1, 4 },
	/* 0xB8 */ { "cp b", 1, 4 },
	/* 0xB9 */ { "cp c", 1, 4 },
	/* 0xBA */ { "cp d", 1, 4 },
	/* 0xBB */ { "cp e", 1, 4 },
	/* 0xBC */ { "cp h", 1, 4 },
	/* 0xBD */ { "cp l", 1, 4 },
	/* 0xBE */ { "cp (hl)", 1, 8 },
	/* 0xBF */ { "cp a", 1, 4 },
	/* 0xC0 */ { "ret nz", 1, 8 },
	/* 0xC1 */ { "pop bc", 1, 12 },
	/* 0xC2 */ { "jp nz, a16", 3, 12 },
	/* 0xC3 */ { "jp a16", 3, 16 },
	/* 0xC4 */ { "call nz, a16", 3, 12 },
	/* 0xC5 */ { "push bc", 1, 16 },
	/* 0xC6 */ { "add a, d8", 2, 8 },
	/* 0xC7 */ { "rst 00h", 1, 16 },
	/* 0xC8 */ { "ret z", 1, 8 },
	/* 0xC9 */ { "ret", 1, 16 },
	/* 0xCA */ { "jp z, a16", 3, 12 },
	/* 0xCB */ { "prefix cb", 1, 4 },
	/* 0xCC */ { "call z, a16", 3, 12 },
	/* 0xCD */ { "call a16", 3, 24 },
	/* 0xCE */ { "adc a, d8", 2, 8 },
	/* 0xCF */ { "rst 08h", 1, 16 },
	/* 0xD0 */ { "ret nc", 1, 8 },
	/* 0xD1 */ { "pop de", 1, 12 },
	/* 0xD2 */ { "jp nc, a16", 3, 12 },
	/* 0xD3 */ { "unknown", 1, 0 },
	/* 0xD4 */ { "call nc, a16", 3, 12 },
	/* 0xD5 */ { "push de", 1, 16 },
	/* 0xD6 */ { "sub d8", 2, 8 },
	/* 0xD7 */ { "rst 10h", 1, 16 },
	/* 0xD8 */ { "ret c", 1, 8 },
	/* 0xD9 */ { "reti", 1, 16 },
	/* 0xDA */ { "jp c, a16", 3, 12 },
	/* 0xDB */ { "unknown", 1, 0 },
	/* 0xDC */ { "call c, a16", 3, 12 },
	/* 0xDD */ { "unknown", 1, 0 },
	/* 0xDE */ { "sbc a, d8", 2, 8 },
	/* 0xDF */ { "rst 18h", 1, 16 },
	/* 0xE0 */ { "ldh (a8), a", 2, 12 },
	/* 0xE1 */ { "pop hl", 1, 12 },
	/* 0xE2 */ { "ld (c), a", 1, 8 },
	/* 0xE3 */ { "unknown", 1, 0 },
	/* 0xE4 */ { "unknown", 1, 0 },
	/* 0xE5 */ { "push hl", 1, 16 },
	/* 0xE6 */ { "and d8", 2, 8 },
	/* 0xE7 */ { "rst 20h", 1, 16 },
	/* 0xE8 */ { "add sp, r8", 2, 16 },
	/* 0xE9 */ { "jp (hl)", 1, 4 },
	/* 0xEA */ { "ld (a16), a", 3, 16 },
	/* 0xEB */ { "unknown", 1, 0 },
	/* 0xDC */ { "unknown", 1, 0 },
	/* 0xDD */ { "unknown", 1, 0 },
	/* 0xEE */ { "xor d8", 2, 8 },
	/* 0xEF */ { "rst 28h", 1, 16 },
	/* 0xF0 */ { "ldh a, (a8)", 2, 12 },
	/* 0xF1 */ { "pop af", 1, 12 },
	/* 0xF2 */ { "ld a, (c)", 1, 8 },
	/* 0xF3 */ { "di", 1, 4 },
	/* 0xF4 */ { "unknown", 1, 0 },
	/* 0xF5 */ { "push af", 1, 16 },
	/* 0xF6 */ { "or d8", 2, 8 },
	/* 0xF7 */ { "rst 30h", 1, 16 },
	/* 0xF8 */ { "ld hl, sp+r8", 2, 12 },
	/* 0xF9 */ { "ld sp, hl", 1, 8 },
	/* 0xFA */ { "ld a, (a16)", 3, 16 },
	/* 0xFB */ { "ei", 1, 4 },
	/* 0xFC */ { "unknown", 1, 0 },
	/* 0xFD */ { "unknown", 1, 0 },
	/* 0xFE */ { "cp d8", 2, 8 },
	/* 0xFF */ { "rst 38h", 1, 16 }
};