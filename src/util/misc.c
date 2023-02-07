#include <string.h>
#include <stdlib.h>

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

// A impl of C23 strdup()
char* strclone(char *s) {
	int n = strlen(s);
	char *res = malloc(n + 1);
	memcpy(res, s, n * sizeof(char));
	res[n] = '\0';
	return res;
}

