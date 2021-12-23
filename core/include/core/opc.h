
#ifndef OPC_H
#define OPC_H

#include "util.h"

typedef struct opc
{
	char* disasm;
	usize length;
	usize cycles;
} opc_t;

extern opc_t opc_opcodes[256];
extern opc_t opc_opcodes_cb[256];

#endif
