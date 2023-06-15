#ifndef ACC_VTYPE_H
#define ACC_VTYPE_H

#include <stdbool.h>

// Defination of first-class types.
enum {
	VT_VOID,	// void
	VT_BOOL,	// bool
	VT_I32,		// int32_t
	VT_I64,		// int64_t

	// Guard
	VT_EXCEED
};

// Value type in C.
struct VType {
	int bt;		// base type(first class)
};

// Find out the type after appling the give ast operator(unary arithmetic variant).
// Writes into parameter _res_
void VType_unary(const struct VType *self, int op, struct VType *res, int line);

// Initialize a VType.
void VType_init(struct VType *self);

// Returns the number of bits in a int type.
// Returns 0 if the given value type is not a variant of int type.
int VType_int_size(const struct VType *self);

// Returns whether the first given value type can be extended to be equal second.
bool VType_ext_eq(const struct VType *x, const struct VType *y);

// Returns whether the given value types are the same.
bool VType_eq(const struct VType *x, const struct VType *y);

#endif
