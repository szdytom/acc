#ifndef ACC_ACIR_H
#define ACC_ACIR_H

// operations in the ACC IR(ACIR)
enum {
	IR_ADD, IR_SUB, IR_MUL, IR_DIV,
	IR_EQ, IR_NE, IR_LT, IR_GT, IR_LE, IR_GE,
	IR_NEG,
	IR_LIT,
};

struct IRinstruction {
	int op;
	union {
		struct { IRinstruction *left, *right; };
		int val;
	};
};


#endif