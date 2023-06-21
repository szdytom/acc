#ifndef ACC_TARGET_H
#define ACC_TARGET_H

#include "vtype.h"

// Defination of targets.
enum {
	TARGET_X86_64,		// Intel/AMD x86-64
	TARGET_X86_32,		// Intel/AMD x86-32
	TARGET_UNKNOWN_16,	// unspecified 16bit instruction set
	TARGET_UNKNOWN_32,	// unspecified 32bit instruction set
	TARGET_RISCV_32,	// RISC-V 32 bits (32 registers)
	TARGET_RISCV_64,	// RISC-V 64 bits (32 registers)

	// Guard
	TARGET_NULL,
};

// Target archtechture infomation.
struct target_info {
	int int_size;		// size of int(in bytes).
	int long_size;		// size of long(in bytes).
};

extern struct target_info Tinfo;

int target_parse(const char *target_string);
void Tinfo_load(int target);

#endif
