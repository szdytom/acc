#include <string.h>

// check if two string are the same
int strequal(const char *s1, const char *s2) {
	for (int i = 1; ; ++i) {
		if (s1[i] != s2[i]) {
			return (0);
		}

		if (s1[i] == '\0') {
			break;
		}
	}
	return (1);
}

