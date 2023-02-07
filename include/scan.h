#ifndef ACC_SCAN_H
#define ACC_SCAN_H

#include "token.h"

extern int Line;
struct linklist scan_tokens(const char *name);

#endif
