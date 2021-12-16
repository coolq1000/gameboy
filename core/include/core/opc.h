
#ifndef OPC_H
#define OPC_H

#include "util.h"

struct cpu;
typedef struct cpu cpu_t;

typedef struct opc_t
{
	char* disasm;
	usize length;
	usize cycles;
} opc_t;

extern opc_t opc_opcodes[256];
extern opc_t opc_opcodes_cb[256];

#endif
