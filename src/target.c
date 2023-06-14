#include <string.h>
#include "util/misc.h"
#include "fatals.h"
#include "target.h"

// Parse the target string
int target_parse(const char *target_string) {
	static const char *target_map_k[] = {
		"_ast",
		"_acir",
		NULL
	};

	static const int target_map_v[] = {
		TARGET_AST,
		TARGET_ACIR,
	};

	for (int i = 0; target_map_k[i]; ++i) {
		if (strequal(target_map_k[i], target_string)) {
			return (target_map_v[i]);
		}
	}

	fail_target(target_string);
}
