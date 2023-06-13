#ifndef ACC_UTIL_MISC_H
#define ACC_UTIL_MISC_H

#include <stdbool.h>

#define ACC_ARRAY_LENGTH(a) (sizeof((a))/sizeof(*(a)))

bool strequal(const char *s1, const char *s2);
char* strclone(const char *s);

#endif
