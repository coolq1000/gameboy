
#ifndef OPC_H
#define OPC_H

#include <stdint.h>
#include <stddef.h>

struct cpu;
typedef struct cpu cpu_t;

typedef struct
{
	char* disasm;
	size_t length;
	size_t cycles;
} opc_t;

extern opc_t opc_opcodes[256];
extern opc_t opc_opcodes_cb[256];

#endif
