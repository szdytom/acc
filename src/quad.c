#include <stdio.h>
#include <stdlib.h>
#include "quad.h"
#include "fatals.h"

// Constructs a Quad with an operator, a destination and two operands.
struct Quad* quad_make(int op, struct Qvar *dest, struct Qvar *left, struct Qvar *right) {
	struct Quad *x = malloc_or_fail(sizeof(struct Quad), __FUNCTION__);
	
	x->op = op;
	x->dest = dest;
	x->left = left;
	x->right = right;
	return (x);
}

// Constructs a Quad with a loads an integer immediate (32bits).
struct Quad* quad_make_i32(struct Qvar *dest, int32_t v) {
	struct Quad *x = malloc_or_fail(sizeof(struct Quad), __FUNCTION__);

	x->op = Q_IMM_I32;
	x->dest = dest;
	x->val_i32 = v;
	return (x);
}

// Constructs a Quad with instruction Q_BR_XXX (conditional goto).
struct Quad* quad_make_br(int op, struct Qblock *bt, struct Qblock *bf) {
	struct Quad *x = malloc_or_fail(sizeof(struct Quad), __FUNCTION__);
	
	x->op = op;
	x->bt = bt;
	x->bf = bf;
	return (x);
}

// Returns whether the given opcode is a opcode from a terminate.
bool quad_is_terminate(int op) {
	switch (op) {
		case Q_RET:
		case Q_BR_ALWAYS:
			return (true);

		default:
			return (false);
	}
}

// Constructs a Qblock.
struct Qblock* qblock_make(int id) {
	struct Qblock *x = malloc_or_fail(sizeof(struct Qblock), __FUNCTION__);
	llist_init(&x->ins);
	x->id = id;
	x->is_complete = false;
	return (x);
}

// Allocates a new basic block in the give function.
struct Qblock* qfunc_new_block(struct Qfunction *f) {
	struct Qblock *x = qblock_make(f->bs.length);
	llist_pushback(&f->bs, x);
	return (x);
}

// Appends an instruction to a block.
void qblock_add_ins(struct Qblock *b, struct Quad *x) {
	if (b->is_complete) {
		return;
	}
	llist_pushback(&b->ins, x);
}

// Constructs a new Qvar.
struct Qvar* qvar_make(int id) {
	struct Qvar *res = malloc_or_fail(sizeof(struct Qvar), __FUNCTION__);
	res->id = id;
	return (res);
}

// Allocates a new variable in the given function.
struct Qvar* qfunc_new_var(struct Qfunction *f) {
	struct Qvar *res = qvar_make(f->vars.length);
	llist_pushback(&f->vars, res);
	return (res);
}

// Translate an AST unary arithmetic opcode to a Quad opcode.
static int unary_arithop(int op) {
	switch (op) {
		case A_NEG:	return (Q_NEG);
		case A_BNOT:	return (Q_NOT);
		default:	fail_quad_op(op, __FUNCTION__);
	}
}

static struct Qvar* qcg_dfs(struct ASTnode *x, struct Qfunction *f, struct Qblock *b) {
	if (x == NULL) {
		return (NULL);
	}

	switch (x->op) {
		case A_RETURN: {
			struct ASTunnode *t = (void*)x;
			struct Qvar *value = qcg_dfs(t->left, f, b);
			qblock_add_ins(b, quad_make(Q_RET, NULL, value, NULL));
			b->is_complete = true;
			return (NULL);
		}

		case A_BLOCK: {
			struct ASTblocknode *t = (void*)x;
			struct llist_node *p = t->st.head;
			while (p) {
				qcg_dfs((struct ASTnode*)p, f, b);
				p = p->nxt;
			}
			return (NULL);
		}

		case A_LIT_I32: {
			struct Qvar *res = qfunc_new_var(f);
			struct ASTi32node *t = (void*)x;
			qblock_add_ins(b, quad_make_i32(res, t->val));
			return (res);
		}

		case A_NEG: case A_BNOT: {
			struct Qvar *res = qfunc_new_var(f);
			struct ASTunnode *t = (void*)x;
			struct Qvar *value = qcg_dfs(t->left, f, b);
			qblock_add_ins(b, quad_make(unary_arithop(x->op), res, value, NULL));
			return (res);
		}

		case A_LNOT: {
			struct Qvar *res = qfunc_new_var(f);
			struct ASTunnode *t = (void*)x;
			struct Qvar *value = qcg_dfs(t->left, f, b);
			struct Qvar *zero = qfunc_new_var(f);
			qblock_add_ins(b, quad_make_i32(zero, 0));
			qblock_add_ins(b, quad_make(Q_CMP_EQ, res, value, zero));
			return (res);
		}

		default: {
			fail_ast_op(x->op, __FUNCTION__);
		}
	}
}

// Generates Quad Repersentation from an AST
struct Qfunction* qfunc_cgenerate(struct Afunction *afunc) {
	struct Qfunction *self = malloc_or_fail(sizeof(struct Qfunction), __FUNCTION__);

	self->name = afunc->name;	// transfer ownership of function name string
	afunc->name = NULL;		// prevents the pointer being freed when freeing the Afunction

	llist_init(&self->bs);
	llist_init(&self->vars);

	struct Qblock *entry = qfunc_new_block(self);
	qcg_dfs(afunc->rt, self, entry);
	return (self);
}

// Frees a Qblock and all its components.
void qblock_free(struct Qblock *b) {
	llist_free(&b->ins);
	free(b);
}

// Frees a Qfunction and all its components.
void qfunc_free(struct Qfunction *f) {
	if (f->name) {
		free(f->name);
	}

	struct llist_node *p = f->bs.head, *nxt;
	while (p) {
		nxt = p->nxt;
		qblock_free((struct Qblock*)p);
		p = nxt;
	}

	llist_free(&f->vars);
	free(f);
}

// Prints the given instruction for debugging.
static void quad_debug_print(struct Quad *self, FILE *Outfile) {
	switch(self->op) {
		case Q_IMM_I32: {
			fprintf(Outfile, "\t$%d = i32 %d;\n", self->dest->id, self->val_i32);
		}	break;

		case Q_RET: {
			if (self->left) {
				fprintf(Outfile, "\tret $%d.\n", self->left->id);
			} else {
				fputs("\tret.", Outfile);
			}
		}	break;

		case Q_NEG: {
			fprintf(Outfile, "\t$%d = neg $%d;\n", self->dest->id, self->left->id);
		}	break;

		case Q_NOT: {
			fprintf(Outfile, "\t$%d = not $%d;\n", self->dest->id, self->left->id);
		}	break;

		case Q_CMP_EQ: {
			fprintf(Outfile, "\t$%d = eq $%d, $%d;\n", self->dest->id, self->left->id, self->right->id);
		}	break;

		default: {
			fail_quad_op(self->op, __FUNCTION__);
		}
	}
}

// Prints the contents of a Qblock for debugging.
static void qblock_debug_print(struct Qblock *self, FILE *Outfile) {
	fprintf(Outfile, "L%d:\n", self->id);
	struct llist_node *p = self->ins.head;
	while (p) {
		quad_debug_print((struct Quad*)p, Outfile);
		p = p->nxt;
	}
}

// Prints the contents of a Qfunction for debugging.
void qfunc_debug_print(struct Qfunction *self, FILE *Outfile) {
	fprintf(Outfile, "%s:\n", self->name);
	struct llist_node *p = self->bs.head;
	while (p) {
		qblock_debug_print((struct Qblock*)p, Outfile);
		p = p->nxt;
	}
}
