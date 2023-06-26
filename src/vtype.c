#include "vtype.h"
#include "ast.h"
#include "fatals.h"

// Find out the type after appling the give ast operator(unary arithmetic variant).
// Writes into parameter _res_
void VType_unary(const struct VType *self, int op, struct VType *res, int line) {
	if (op == A_RETURN) {
		VType_init(res);
		return;
	}

	*res = *self;
	if (self->bt == VT_VOID) {
		fail_type(line);
	}

	switch (op) {
		case A_BNOT:
		case A_NEG: {
		}	break;

		case A_LNOT: {
			res->bt = VT_BOOL;
		}	break;

		default: {
			fail_ast_op(op, __FUNCTION__);
		}
	}
}

// Initialize a VType into void.
void VType_init(struct VType *self) {
	self->bt = VT_VOID;
}

// Returns the number of bits in a int type.
// Returns 0 if the given value type is not a variant of int type.
int VType_int_size(const struct VType *self) {
	static const int map[][2] = {
		{VT_BOOL,	1},
		{VT_I32,	32},
		{VT_I64,	64},
		{VT_EXCEED},
	};

	for (int i = 0; map[i][0] != VT_EXCEED; ++i) {
		if (map[i][0] == self->bt) {
			return (map[i][1]);
		}
	}

	return (0);
}

// Returns whether the first given value type can be extended to be equal second.
bool VType_ext_eq(const struct VType *x, const struct VType *y) {
	if (VType_eq(x, y)) {
		return (true);
	}

	int xsz = VType_int_size(x);
	if (xsz == 0) {
		return (false);
	}

	int ysz = VType_int_size(y);
	if (ysz == 0) {
		return (false);
	}
	return (xsz <= ysz);
}

// Returns whether the given value types are the same.
bool VType_eq(const struct VType *x, const struct VType *y) {
	return (x->bt == y->bt);
}

// Returns whether the given VType is a variant of integer(including bool)
bool VType_is_int(const struct VType *self) {
	switch (self->bt) {
		case VT_BOOL: case VT_I32: case VT_I64:
			return (true);
		default:
			return (false);
	}
}