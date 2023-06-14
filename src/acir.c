#include <stdlib.h>
#include "util/misc.h"
#include "fatals.h"
#include "acir.h"

#define IRinstruction_constructor_shared_code \
	struct IRinstruction *self = malloc_or_fail(sizeof(struct IRinstruction), __FUNCTION__);	\
	self->id = IRfunction_alloc_ins(owner->owner);							\
	self->owner = owner;										\
	IRblock_add_ins(owner, self);									\

// Adds one instruction to list.
// Internal function only: IRinstruction_new_xxx() automaticly calls this function.
static void IRblock_add_ins(struct IRblock *self, struct IRinstruction *x) {
	if (self->is_complete) {
		return;
	}
	llist_pushback(&self->ins, x);
}

// Constructs an IRinstruction with an operator, and two operands.
struct IRinstruction* IRinstruction_new(struct IRblock *owner, int op, 
					struct IRinstruction *left, struct IRinstruction *right) {
	IRinstruction_constructor_shared_code

	self->op = op;
	self->left = left;
	self->right = right;
	
	if (IRis_terminate(self->op)) {
		owner->is_complete = true;
	}
	return (self);
}

// Constructs a IRinstruction with an integer immediate (32bits).
struct IRinstruction* IRinstruction_new_i32(struct IRblock *owner, int32_t v) {
	IRinstruction_constructor_shared_code

	self->op = IR_IMM_I32;
	self->val_i32 = v;
	return (self);
}

// Constructs a IRinstuction with instruction Q_JMP or Q_BR (which is conditional jump).
struct IRinstruction* IRinstruction_new_jmp(struct IRblock *owner, int op, struct IRinstruction *cond,
						struct IRblock *bt, struct IRblock *bf) {
	IRinstruction_constructor_shared_code

	self->op = op;
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
	struct IRblock *self = malloc_or_fail(sizeof(struct IRblock), __FUNCTION__);

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

// Translate an AST unary arithmetic opcode to a IR opcode.
static int translate_ast_unary_op(int op) {
	switch (op) {
		case A_NEG:	return (IR_NEG);
		case A_BNOT:	return (IR_NOT);
		default:	fail_ir_op(op, __FUNCTION__);
	}
}

static struct IRinstruction* IRcg_dfs(struct ASTnode *x, struct IRfunction *f, struct IRblock *b) {
	if (x == NULL) {
		return (NULL);
	}

	switch (x->op) {
		case A_RETURN: {
			struct ASTunnode *t = (void*)x;
			struct IRinstruction *value = IRcg_dfs(t->left, f, b);
			IRinstruction_new(b, IR_RET, value, NULL);
			b->is_complete = true;
			return (NULL);
		}

		case A_BLOCK: {
			struct ASTblocknode *t = (void*)x;
			struct llist_node *p = t->st.head;
			while (p) {
				IRcg_dfs((struct ASTnode*)p, f, b);
				p = p->nxt;
			}
			return (NULL);
		}

		case A_LIT_I32: {
			struct ASTi32node *t = (void*)x;
			return (IRinstruction_new_i32(b, t->val));
		}

		case A_NEG: case A_BNOT: {
			struct ASTunnode *t = (void*)x;
			struct IRinstruction *value = IRcg_dfs(t->left, f, b);
			return (IRinstruction_new(b, translate_ast_unary_op(x->op), value, NULL));
		}

		case A_LNOT: {
			struct ASTunnode *t = (void*)x;
			struct IRinstruction *value = IRcg_dfs(t->left, f, b),
					     *zero = IRinstruction_new_i32(b, 0);
			return (IRinstruction_new(b, IR_CMP_EQ, value, zero));
		}

		default: {
			fail_ast_op(x->op, __FUNCTION__);
		}
	}
}

// Generates Quad Repersentation from an AST
struct IRfunction* IRfunction_from_ast(struct Afunction *afunc) {
	struct IRfunction *self = malloc_or_fail(sizeof(struct IRfunction), __FUNCTION__);

	self->name = afunc->name;	// transfer ownership of function name string
	afunc->name = NULL;		// prevents the pointer being freed when freeing the Afunction

	self->ins_count = 0;
	llist_init(&self->bs);

	struct IRblock *entry = IRblock_new(self);
	IRcg_dfs(afunc->rt, self, entry);
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
		case IR_IMM_I32: {
			fprintf(Outfile, "\t$%d = i32 %d;\n", self->id, self->val_i32);
		}	break;

		case IR_RET: {
			if (self->left) {
				fprintf(Outfile, "\tret $%d.\n", self->left->id);
			} else {
				fputs("\tret.", Outfile);
			}
		}	break;

		case IR_NEG: {
			fprintf(Outfile, "\t$%d = neg $%d;\n", self->id, self->left->id);
		}	break;

		case IR_NOT: {
			fprintf(Outfile, "\t$%d = not $%d;\n", self->id, self->left->id);
		}	break;

		case IR_CMP_EQ: {
			fprintf(Outfile, "\t$%d = eq $%d, $%d;\n", self->id, self->left->id, self->right->id);
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
