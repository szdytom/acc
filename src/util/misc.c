#include <string.h>
#include <stdlib.h>
#include "util/misc.h"

// This function does what you think it does :)
// Returns whether the given strings are the same.
bool strequal(const char *s1, const char *s2) {
	return (strcmp(s1, s2) == 0);
}

// A impl of C23 strdup().
// Clones the given string and returns a pointer.
char* strclone(const char *s) {
	int n = strlen(s);
	char *res = malloc(n + 1);
	memcpy(res, s, n * sizeof(char));
	res[n] = '\0';
	return res;
}
