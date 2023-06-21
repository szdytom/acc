#include <string.h>
#include "util/misc.h"
#include "fatals.h"
#include "target.h"

// Parse the target string
int target_parse(const char *target_string) {
	static const char *target_map_k[] = {
		"x86_64",
		"x86",
		"unknown16",
		"unknown32",
		"riscv_32",
		"riscv_64",
		NULL
	};

	static const int target_map_v[] = {
		TARGET_X86_64,
		TARGET_X86_32,
		TARGET_UNKNOWN_16,
		TARGET_UNKNOWN_32,
		TARGET_RISCV_32,
		TARGET_RISCV_64,
	};

	for (int i = 0; target_map_k[i]; ++i) {
		if (strequal(target_map_k[i], target_string)) {
			return (target_map_v[i]);
		}
	}

	fail_target(target_string);
}

struct target_info Tinfo;

void Tinfo_load(int target) {
	static struct target_info map[] = {
	{	// x86-64
		.int_size = 4,
		.long_size = 8,
	}, {	// x84
		.int_size = 4,
		.long_size = 4,
	}, {	// unknown16
		.int_size = 2,
		.long_size = 2,
	}, {	// unknown32
		.int_size = 4,
		.long_size = 4,
	}, {	// riscv_32
		.int_size = 4,
		.long_size = 4,
	}, {	// riscv_64
		.int_size = 4,
		.long_size = 8
	}};

	if (target < 0 || target >= TARGET_NULL) {
		fail_unreachable(__FUNCTION__);
	}
	Tinfo = map[target];
}