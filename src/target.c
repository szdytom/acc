#include <string.h>
#include "fatals.h"
#include "target.h"

int target_parse(const char *target_string) {
	static const char *target_map_k[] = {
		"ast",
		NULL
	};

	static const int target_map_v[] = {
		TARGET_AST,
	};

	for (int i = 0; target_map_k[i]; ++i) {
		if (strcmp(target_map_k[i], target_string) == 0) {
			return (target_map_v[i]);
		}
	}

	fail_target(target_string);
}
