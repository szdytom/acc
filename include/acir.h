#ifndef ACC_ACIR_H
#define ACC_ACIR_H

#include <stdbool.h>
#include <stdint.h>
#include "ast.h"
#include "util/linklist.h"

// Operation code definations in the ACC IR(ACIR).
enum {
	// Loads
	IR_IMM_I32,	// immediate integer (32bits)

	// SSA
	IR_PHI,		// phi node in SSA

	// Arithmetic operations
	IR_NEG,		// negation
	IR_NOT,		// bitwise not
	IR_CMP_EQ,	// compare whether equal

	// Terminates
	IR_RET,		// return 
	IR_JMP,		// jump: always goto true branch
	IR_BR,		// conditional jump

	// Guard
	IR_NULL,
};

// Argument of phi function.
struct IRphi_arg {
	struct llist_node n;		// linklist header
	struct IRblock *source;		// predecessor basic block 
	struct IRinstruction *value;	// corresponding value when comming from the block
};

// IR instruction.
// The result is represented as itself.
struct IRinstruction {
	struct llist_node n;	// linklist header
	int op;			// operation code
	int id;			// value identifier
	struct IRblock *owner;	// the basic block containing this instruction
	union {
		struct { struct IRinstruction *left, *right; };	// left/right operands for calculations
		struct { struct IRinstruction *cond; 		// jump condition
			 struct IRblock *bt, *bf; };		// true branch & false branch for conditional jump
		struct linklist phi;				// Phi instruction argument list
		int32_t val_i32;				// immediate: integer (32bits)
	};
};

// IR basic block
struct IRblock {
	struct llist_node n;		// linklist header
	int id;				// block identifier, the function entry block will have value 0
	struct linklist ins;		// contained instructions
	bool is_complete;		// whether the block is properly ended with a terminate
	struct linklist pre;		// predecessor list of this basic block
	struct IRfunction *owner;	// the function containing this function
};

// Function containing IR instructions.
// TODO: paramaters
struct IRfunction {
	struct llist_node n;	// linklist header
	char *name;		// function name
	struct linklist bs;	// basic blocks
	int ins_count;		// number of instructions, used for allocating instruction identifier.
};

// Constructs an IRinstruction with an operator, and two operands.
struct IRinstruction* IRinstruction_new(struct IRblock *owner, int op, 
					struct IRinstruction *left, struct IRinstruction *right);

// Constructs a IRinstruction with an integer immediate (32bits).
struct IRinstruction* IRinstruction_new_i32(struct IRblock *owner, int32_t v);

// Constructs a IRinstuction with instruction Q_JMP or Q_BR (which is conditional jump).
struct IRinstruction* IRinstruction_new_jmp(struct IRblock *owner, int op, struct IRinstruction *cond,
						struct IRblock *bt, struct IRblock *bf);

// Returns whether an IR opcode is a terminate.
// Terminate must and may only appear exactly once at ther end of each basic block.
bool IRis_terminate(int op);

// Returns whether an IR opcode is a jump opcode.
bool IRis_jmp(int op);

// Constructs a IRblock.
struct IRblock* IRblock_new(struct IRfunction *owner);

// Allocating instruction identifiers in IRfunction
int IRfunction_alloc_ins(struct IRfunction *self);

// Generates Quad Repersentation from an AST
struct IRfunction* IRfunction_from_ast(struct Afunction *afunc);

// Frees a IRinstruction and all its components.
void IRinstruction_free(struct IRinstruction *self);

// Frees a IRblock and all its components.
void IRblock_free(struct IRblock *self);

// Frees a IRfunction and all its components.
void IRfunction_free(struct IRfunction *self);

// Outputs the instruction.
void IRinstruction_print(struct IRinstruction *self, FILE *Outfile);

// Outputs the containing instructions of the IRblock.
void IRblock_print(struct IRblock *self, FILE *Outfile);

// Outputs the containing instructions of the IRfunction.
void IRfunction_print(struct IRfunction *self, FILE *Outfile);

#endif
