#include <stdlib.h>
#include "util/misc.h"
#include "fatals.h"
#include "acir.h"

#define IRinstruction_constructor_shared_code \
	struct IRinstruction *self = try_malloc(sizeof(struct IRinstruction), __FUNCTION__);	\
	self->id = IRfunction_alloc_ins(owner->owner);						\
	self->owner = owner;									\
	IRblock_add_ins(owner, self);								\

// Adds one instruction to list.
// Internal function only: IRinstruction_new_xxx() automaticly calls this function.
static void IRblock_add_ins(struct IRblock *self, struct IRinstruction *x) {
	if (self->is_complete) {
		return;
	}
	llist_pushback(&self->ins, x);
}

// Constructs an IRinstruction with an operator, and two operands.
struct IRinstruction* IRinstruction_new(struct IRblock *owner, int op, int type,
					struct IRinstruction *left, struct IRinstruction *right) {
	IRinstruction_constructor_shared_code

	self->op = op;
	self->type = type;
	self->left = left;
	self->right = right;

	if (IRis_terminate(self->op)) {
		owner->is_complete = true;
	}
	return (self);
}

// Constructs an IRinstruction with an integer immediate (32bits).
struct IRinstruction* IRinstruction_new_i32(struct IRblock *owner, int32_t v) {
	IRinstruction_constructor_shared_code

	self->op = IR_IMM;
	self->type = IRT_I32;
	self->val_i32 = v;
	return (self);
}

// Contructs an IRinstruction with an void immediate only.
// Use IRfunction.null instead of contructing a new void immediate.
struct IRinstruction* IRinstruction_new_void(struct IRblock *owner) {
	IRinstruction_constructor_shared_code

	self->op = IR_IMM;
	self->type = IRT_VOID;
	return (self);
}

// Constructs a IRinstuction with instruction IR_JMP or IR_BR (which is conditional jump).
struct IRinstruction* IRinstruction_new_jmp(struct IRblock *owner, int op, struct IRinstruction *cond,
						struct IRblock *bt, struct IRblock *bf) {
	IRinstruction_constructor_shared_code

	self->op = op;
	self->type = IRT_VOID;
	self->cond = cond;
	self->bt = bt;
	self->bf = bf;
	owner->is_complete = true;

	if (bt) {
		llist_pushback(&bt->pre, owner);
	}
	if (bf) {
		llist_pushback(&bf->pre, owner);
	}
	return (self);
}

// Returns whether an IR opcode is a terminate.
// Terminate must and may only appear exactly once at ther end of each basic block.
bool IRis_terminate(int op) {
	switch (op) {
		case IR_RET: case IR_BR: case IR_JMP:
			return (true);

		default:
			return (false);
	}
}

// Returns whether an IR opcode is a jump opcode.
bool IRis_jmp(int op) {
	switch (op) {
		case IR_JMP: case IR_BR:
			return (true);

		default:
			return (false);
	}
}

// Constructs a IRblock.
struct IRblock* IRblock_new(struct IRfunction *owner) {
	struct IRblock *self = try_malloc(sizeof(struct IRblock), __FUNCTION__);

	self->id = owner->bs.length;
	self->owner = owner;
	self->is_complete = false;
	llist_init(&self->pre);
	llist_init(&self->ins);
	llist_pushback(&owner->bs, self);
	return (self);
}

// Allocating instruction identifiers in IRfunction
int IRfunction_alloc_ins(struct IRfunction *self) {
	return self->ins_count++;
}

// Translates a VType into an IR type code.
int IRTypecode_from_VType(const struct VType *v) {
	int map[][2] = {
		{VT_VOID,	IRT_VOID},
		{VT_BOOL,	IRT_I1},
		{VT_I32,	IRT_I32},
		{VT_I64,	IRT_I64},
		{VT_EXCEED}
	};

	for (int i = 0; map[i][0] != VT_EXCEED; ++i) {
		if (map[i][0] == v->bt) {
			return (map[i][1]);
		}
	}

	fail_unreachable(__FUNCTION__);
}

int IRTypecode_integer_promote(int self) {
	switch(self) {
		case IRT_I1: case IRT_I32:
			return (IRT_I32);
		case IRT_I64:
			return (IRT_I64);
		default:
			fail_unreachable(__FUNCTION__);
	}
}

// Returns a string identifier for the given type.
const char *IRTypecode_stringify(int self) {
	static const char *map[] = {
		"void",
		"i1",
		"i32",
		"i64",
		"ptr",
		NULL
	};

	return map[self];
}

// Translate an AST unary arithmetic opcode to a IR opcode.
static int IRopcode_from_ast_unary(int op) {
	switch (op) {
		case A_NEG:	return (IR_NEG);
		case A_BNOT:	return (IR_NOT);
		default:	fail_ir_op(op, __FUNCTION__);
	}
}

// Returns a string identifier for the given operation code.
const char* IRopcode_stringify(int self) {
	static const char *map[] = {
		"imm",
		"phi",
		"zext",
		"sext",
		"trunc",
		"neg",
		"not",
		"eq",
		"ret",
		"jmp",
		"br",
		NULL
	};

	return map[self];
}

// DFS on an AST and build IR.
static struct IRinstruction* IRcg_dfs(struct ASTnode *x, struct IRfunction *f, struct IRblock *b) {
	// nothing to do, return the null object.
	if (x == NULL) {
		return (f->null);	
	}

	switch (x->op) {
		case A_RETURN: {
			struct ASTunnode *t = (void*)x;
			struct IRinstruction *value = IRcg_dfs(t->left, f, b);
			IRinstruction_new(b, IR_RET, IRT_VOID, value, NULL);
			b->is_complete = true;
			return (f->null);
		}

		case A_BLOCK: {
			struct ASTblocknode *t = (void*)x;
			struct llist_node *p = t->st.head;
			while (p) {
				IRcg_dfs((struct ASTnode*)p, f, b);
				p = p->nxt;
			}
			return (f->null);
		}

		case A_LIT_I32: {
			struct ASTi32node *t = (void*)x;
			return (IRinstruction_new_i32(b, t->val));
		}

		case A_NEG: case A_BNOT: {
			struct ASTunnode *t = (void*)x;
			struct IRinstruction *value = IRcg_dfs(t->left, f, b);

			int type = IRTypecode_integer_promote(value->type);
			if (type != value->type) {
				value = IRinstruction_new(b, IR_SEXT, type, value, NULL);
			}

			return (IRinstruction_new(b, IRopcode_from_ast_unary(x->op), type, value, NULL));
		}

		case A_LNOT: {
			struct ASTunnode *t = (void*)x;
			// A logical not operation is basicly equivlant to comparing the value to 0.
			struct IRinstruction *value = IRcg_dfs(t->left, f, b),
					     *zero = IRinstruction_new_i32(b, 0);
			return (IRinstruction_new(b, IR_CMP_EQ, IRT_I1, value, zero));
		}

		default: {
			fail_ast_op(x->op, __FUNCTION__);
		}
	}
}

// Generates IR Repersentation from an AST
struct IRfunction* IRfunction_from_ast(struct Afunction *afunc) {
	struct IRfunction *self = try_malloc(sizeof(struct IRfunction), __FUNCTION__);

	self->name = afunc->name;	// transfer ownership of function name string
	afunc->name = NULL;		// prevents the pointer being freed when freeing the Afunction

	self->ins_count = 0;
	llist_init(&self->bs);

	struct IRblock *entry = IRblock_new(self);	// construct the function entry block.
	self->null = IRinstruction_new_void(entry);	// initialize the null object.
	IRcg_dfs(afunc->rt, self, entry);		// generate code by doing a DFS in our AST.
	return (self);
}

// Frees a IRinstruction and all its components.
void IRinstruction_free(struct IRinstruction *self) {
	if (self->op == IR_PHI) {
		llist_free(&self->phi);
	}
	free(self);
}

// Frees a IRblock and all its components.
void IRblock_free(struct IRblock *self) {
	struct llist_node *p = self->ins.head, *nxt;
	while (p) {
		nxt = p->nxt;
		IRinstruction_free((void*)p);
		p = nxt;
	}
	free(self);
}

// Frees a IRfunction and all its components.
void IRfunction_free(struct IRfunction *self) {
	if (self->name) {
		free(self->name);
	}

	struct llist_node *p = self->bs.head, *nxt;
	while (p) {
		nxt = p->nxt;
		IRblock_free((void*)p);
		p = nxt;
	}

	free(self);
}

// Outputs the instruction.
void IRinstruction_print(struct IRinstruction *self, FILE *Outfile) {
	switch(self->op) {
		case IR_IMM: {
			fprintf(Outfile, "\t$%d = %s", self->id, IRTypecode_stringify(self->type));
			switch (self->type) {
				case IRT_VOID: {
				}	break;

				case IRT_I1: {
					fprintf(Outfile, " %s", self->val_i1 ? "true" : "false");
				}	break;

				case IRT_I32: {
					fprintf(Outfile, " %d", self->val_i32);
				}	break;

				case IRT_I64: {
					fprintf(Outfile, " %lld", self->val_i64);
				}	break;
			}
			fputs(";\n", Outfile);
		}	break;

		case IR_RET: {
			fprintf(Outfile, "\tret $%d.\n", self->left->id);
		}	break;

		case IR_SEXT: case IR_ZEXT: case IR_TRUNC:
		case IR_NEG: case IR_NOT: {
			fprintf(Outfile, "\t$%d = %s %s $%d;\n", self->id, 
				IRTypecode_stringify(self->type), IRopcode_stringify(self->op), self->left->id);
		}	break;

		case IR_CMP_EQ: {
			fprintf(Outfile, "\t$%d = %s %s $%d $%d;\n", self->id
				, IRTypecode_stringify(self->type), IRopcode_stringify(self->op), self->left->id
				, self->right->id);
		}	break;

		default: {
			fail_ir_op(self->op, __FUNCTION__);
		}	break;
	}
}

// Outputs the containing instructions of the IRblock.
void IRblock_print(struct IRblock *self, FILE *Outfile) {
	fprintf(Outfile, "L%d:\n", self->id);

	struct llist_node *p = self->ins.head;
	while (p) {
		IRinstruction_print((void*)p, Outfile);
		p = p->nxt;
	}
}

// Outputs the containing instructions of the IRfunction.
void IRfunction_print(struct IRfunction *self, FILE *Outfile) {
	fprintf(Outfile, "%s:\n", self->name);

	struct llist_node *p = self->bs.head;
	while (p) {
		IRblock_print((void*)p, Outfile);
		p = p->nxt;
	}
}
