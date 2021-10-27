
import json

def generate_disasm(mnemonic, opr1, opr2):
	if opr2 is not None: return "{mnemonic} {operand_1}, {operand_2}".format(mnemonic=mnemonic, operand_1=opr1, operand_2=opr2)
	if opr1 is not None: return "{mnemonic} {operand_1}".format(mnemonic=mnemonic, operand_1=opr1)
	else: return "{mnemonic}".format(mnemonic=mnemonic)

def generate_operation():
	return "// OPERATION"

def generate_flags():
	return "// FLAGS"

# def generate_length(length):
# 	return "cpu->registers.pc += {length};".format(length=length)

# def generate_cycles(cycles):
# 	return "cpu->clock.cycles += {cycles};".format(cycles=cycles)

def generate_opc(ordinal, mnemonic, flags, opr1, opr2, length, cycles):
	disasm = generate_disasm(mnemonic.lower(), opr1.lower() if opr1 else opr1, opr2.lower() if opr2 else opr2)

	code = """case 0x{ordinal:02X}: /* {disasm} */
	{operation}
	{flags}
	break;
""".format(ordinal=ordinal, disasm=disasm, operation=generate_operation(), flags=generate_flags())

	entry = " /* 0x{ordinal:02X} */ {{ \"{disasm}\", {length}, {cycles} }},".format(ordinal=ordinal, disasm=disasm, length=length, cycles=cycles)

	return code, entry

with open('opcodes.json') as f:
	opcodes = json.loads(f.read())

	generated = {}

	cases = ''
	table = ''

	# generate unprefixed opcode implementations and table entries
	for opcode in opcodes['unprefixed']:
		definition = opcodes['unprefixed'][opcode]
		generation = generate_opc(int(opcode, 16), definition['mnemonic'], definition['flags_ZNHC'], definition.get('operand1', None), definition.get('operand2', None), definition['bytes'], min(definition['cycles']))
		generated[opcode] = generation

	# sort generated opcodes
	generated = {k: v for k, v in sorted(generated.items(), key=lambda item: int(item[0], 16))}

	# create final code
	for generation in generated.values():
		code, entry = generation
		cases += code
		table += entry + '\n'

	print(cases, table)
