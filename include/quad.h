#ifndef ACC_QUAD_H
#define ACC_QUAD_H

#include <stdint.h>
#include <stdbool.h>
#include "ast.h"
#include "util/linklist.h"

// Operation code definations
enum {
	// Loading immediates
	Q_IMM_I32,	// integer (32bits)

	// Arithmetic operations
	Q_NEG,		// negation
	Q_NOT,		// bitwise not
	Q_CMP_EQ,	// compare whether equal

	// Terminates
	Q_RET,		// return 
	Q_BR_ALWAYS,	// conditional goto: always goto true branch.

	// Guard
	Q_NULL,
};

// Local(in function) variable.
struct Qvar {
	struct llist_node n;	// linklist header
	int id;			// variable id
};

// Quad instruction.
struct Quad {
	struct llist_node n;	// linklist header
	int op;			// operation code
	struct Qvar *dest;	// operation destination
	union {
		struct { struct Qvar *left, *right; };	// left/right operands for calculations
		struct { struct Qblock *bt, *bf; };	// true branch & false branch for conditional goto
		int32_t val_i32;			// immediate: integer (32bits)
	};
};

// Basic block consist of Quad instructions.
struct Qblock {
	struct llist_node n;	// linklist header
	int id;			// block id
	struct linklist ins;	// instruction Quads
	bool is_complete;	// whether the block is properly ended with a terminate
};

// Function containing Quad instructions.
// TODO: paramaters
struct Qfunction {
	struct llist_node n;	// linklist header
	char *name;		// function name
	struct linklist bs;	// basic blocks
	struct linklist vars;	// local variables
};

// Constructs a Quad with an operator, a destination and two operands.
struct Quad* quad_make(int op, struct Qvar *dest, struct Qvar *left, struct Qvar *right);

// Constructs a Quad with a loads an integer immediate (32bits).
struct Quad* quad_make_i32(struct Qvar *dest, int32_t v);

// Constructs a Quad with instruction Q_BR_XXX (conditional goto).
struct Quad* quad_make_br(int op, struct Qblock *bt, struct Qblock *bf);

// Returns whether the given opcode is a opcode from a terminate.
bool quad_is_terminate(int op);

// Constructs a Qblock.
struct Qblock* qblock_make();

// Allocates a new basic block in the give function.
struct Qblock* qfunc_new_block(struct Qfunction *f);

// Appends an instruction to a block.
void qblock_add_ins(struct Qblock *b, struct Quad *x);

// Constructs a new Qvar.
struct Qvar* qvar_make(int id);

// Allocates a new variable in the given function.
struct Qvar* qfunc_new_var(struct Qfunction *f);

// Generates Quad Repersentation from an AST
struct Qfunction* qfunc_cgenerate(struct Afunction *afunc);

// Frees a Qblock and all its components.
void qblock_free(struct Qblock *b);

// Frees a Qfunction and all its components.
void qfunc_free(struct Qfunction *f);

// Prints the contents of a Qfunction for debugging.
void qfunc_debug_print(struct Qfunction *self, FILE *Outfile);

#endif
