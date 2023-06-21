#ifndef ACC_ACIR_H
#define ACC_ACIR_H

#include <stdbool.h>
#include <stdint.h>
#include "ast.h"
#include "util/linklist.h"

// Operation code definations in the ACC IR(ACIR).
enum {
	// Loads
	IR_IMM,		// load immediate

	// SSA
	IR_PHI,		// phi node in SSA

	// Type casts
	IR_ZEXT,	// zero extend an integer
	IR_SEXT,	// signed extend an integer
	IR_TRUNC,	// truncate an integer

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

// Defination of IR type code, which simplier than VType.
enum {
	IRT_VOID,	// void
	IRT_I1,		// bool
	IRT_I32,	// 32bits integer
	IRT_I64,	// 64bits integer
	IRT_PTR,	// pointer

	// Guard
	IRT_EXCEED
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
	int type;		// value type in IR type code
	struct IRblock *owner;	// the basic block containing this instruction
	union {
		struct { struct IRinstruction *left, *right; };	// left/right operands for calculations
		struct { struct IRinstruction *cond; 		// jump condition
			 struct IRblock *bt, *bf; };		// true branch & false branch for conditional jump
		struct linklist phi;				// Phi instruction argument list
		int32_t val_i32;				// immediate: 32bits integer
		int64_t val_i64;				// immediate: 64bits integer
		bool val_i1;					// immediate: bool
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
	struct llist_node n;		// linklist header
	char *name;			// function name
	struct linklist bs;		// basic blocks
	int ins_count;			// number of instructions, used for allocating instruction identifier.
	struct IRinstruction *null;	// an instruction with a value void, considered to have instruction id 0.
					// This is used for the null object patern.
};

// Constructs an IRinstruction with an operator, and two operands.
struct IRinstruction* IRinstruction_new(struct IRblock *owner, int op, int type,
					struct IRinstruction *left, struct IRinstruction *right);

// Constructs an IRinstruction with an integer immediate (32bits).
struct IRinstruction* IRinstruction_new_i32(struct IRblock *owner, int32_t v);

// Contructs an IRinstruction with an void immediate only.
// Use IRfunction.null instead of contructing a new void immediate.
struct IRinstruction* IRinstruction_new_void(struct IRblock *owner);

// Constructs a IRinstuction with instruction IR_JMP or IR_BR (which is conditional jump).
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

// Translates a VType into an IR type code.
int IRTypecode_from_VType(const struct VType *v);

// Returns a string identifier for the given type.
const char *IRTypecode_stringify(int self);

// Returns a string identifier for the given operation code.
const char* IRopcode_stringify(int op);

// Generates IR Repersentation from an AST
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
