#ifndef ACC_TARGET_H
#define ACC_TARGET_H

// Target types
enum {
	TARGET_AST,
	TARGET_QUAD,
	TARGET_NULL,
};

int target_parse(const char *target_string);

#endif