#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "fatals.h"
#include "util/misc.h"
#include "util/linklist.h"

const char *ast_opname[] = {
	"=",
	"neg", "add", "sub", "mul", "div",
	"==", "!=", "<", ">", "<=", ">=",
	"not", "and", "or",
	"~",
	"int32", "int64",
	"var",
	"block",
	"print", "if", "while",
	"return",
	NULL
};

// Constructs a binary AST node
struct ASTnode* ASTbinnode_new(int op, struct ASTnode *left, struct ASTnode *right) {
	struct ASTbinnode *self = try_malloc(sizeof(struct ASTbinnode), __FUNCTION__);

	VType_init(&self->type);
	self->type.bt = VT_VOID; // FIXME: calculate the correct type.
	self->op = op;
	self->left = left;
	self->right = right;
	return ((void*)self);
}

// Make an AST integer literal (32bits) node
struct ASTnode* ASTi32node_new(int32_t v) {
	struct ASTi32node *self = try_malloc(sizeof(struct ASTi32node), __FUNCTION__);

	VType_init(&self->type);
	self->type.bt = VT_I32;
	self->op = A_LIT_I32;
	self->val = v;
	return ((void*)self);
}

// Make an AST integer literal (64bits) node
struct ASTnode* ASTi64node_new(int64_t v) {
	struct ASTi64node *self = try_malloc(sizeof(struct ASTi64node), __FUNCTION__);

	VType_init(&self->type);
	self->type.bt = VT_I64;
	self->op = A_LIT_I64;
	self->val = v;
	return ((void*)self);
}

// Make an AST variable value node
struct ASTnode* ASTvarnode_new(int id) {
	fail_todo(__FUNCTION__);
	struct ASTvarnode *self = try_malloc(sizeof(struct ASTvarnode), __FUNCTION__);

	self->op = A_VAR;
	self->id = id;
	return ((void*)self);
}

// Constructs a unary AST node: only one child.
struct ASTnode* ASTunnode_new(int op, struct ASTnode *child, int line) {
	struct ASTunnode *self = try_malloc(sizeof(struct ASTunnode), __FUNCTION__);

	VType_unary(&child->type, op, &self->type, line);
	self->op = op;
	self->left = child;
	return ((void*)self);
}

// Make a block ast node
struct ASTnode* ASTblocknode_new() {
	struct ASTblocknode *self = try_malloc(sizeof(struct ASTblocknode), __FUNCTION__);

	VType_init(&self->type);
	self->op = A_BLOCK;
	llist_init(&self->st);
	return ((void*)self);
}

// Make a assignment ast node
struct ASTnode* ASTassignnode_new(int op, struct ASTnode *left, struct ASTnode *right) {
	fail_todo(__FUNCTION__);
	struct ASTassignnode *x = try_malloc(sizeof(struct ASTassignnode), __FUNCTION__);

	x->op = op;
	x->left = left;
	x->right = right;
	return ((struct ASTnode*)x);
}

// Make a if statement ast node
struct ASTnode* ASTifnode_new(struct ASTnode *left, struct ASTnode *right, struct ASTnode *cond) {
	fail_todo(__FUNCTION__);
	struct ASTifnode *x = try_malloc(sizeof(struct ASTifnode), __FUNCTION__);

	x->op = A_IF;
	x->left = left;
	x->right = right;
	x->cond = cond;
	return ((void*)x);
}

static void ast_print_dfs(FILE* Outfile, struct ASTnode *x, int tabs) {
	for (int i = 0; i < tabs; ++i) {
		fprintf(Outfile, "\t");
	}

	if (x == NULL) {
		fprintf(Outfile, "--->NULL.\n");
		return;
	}

	switch(x->op) {
		case A_LNOT: case A_BNOT: case A_NEG:
		case A_RETURN: case A_PRINT: {
			struct ASTunnode *t = (struct ASTunnode*)x;
			fprintf(Outfile, "--->UNOP(%s)\n", ast_opname[x->op]);
			ast_print_dfs(Outfile, t->left, tabs + 1);
		}	break;

		case A_LIT_I32: {
			struct ASTi32node *t = (struct ASTi32node*)x;
			fprintf(Outfile, "--->INT32(%d)\n", t->val);
		}	break;

		case A_LIT_I64: {
			struct ASTi64node *t = (struct ASTi64node*)x;
			fprintf(Outfile, "--->INT64(%lld)\n", t->val);
		}	break;

		case A_BLOCK: {
			struct ASTblocknode *t = (struct ASTblocknode*)x;
			fprintf(Outfile, "--->BLOCK(%d statements)\n", t->st.length);
			struct llist_node *p = t->st.head;
			while (p) {
				ast_print_dfs(Outfile, (struct ASTnode*)p, tabs + 1);
				p = p->nxt;
			}
		}	break;

		default: {
			fail_ast_op(x->op, __FUNCTION__);
		}	break;
	}
}

// Prints the structure of a AST into Outfile.
void ASTnode_print(FILE *Outfile, struct ASTnode *rt) {
	ast_print_dfs(Outfile, rt, 0);
}

// Prints the structure of a Afunction into Outfile.
void Afunction_print(FILE *Outfile, struct Afunction *f) {
	fprintf(Outfile, "FUNCTION %s: \n", f->name);
	ast_print_dfs(Outfile, f->rt, 0);
}

// Constructs a Afunction.
struct Afunction* Afunction_new() {
	struct Afunction *res = (void*)try_malloc(sizeof(struct Afunction), __FUNCTION__);

	res->rt = NULL;
	res->name = NULL;
	return res;
}

// Frees a Afunction and all its components.
void Afunction_free(struct Afunction *f) {
	if (f->name) {
		free(f->name);
	}

	if (f->rt) {
		ASTnode_free(f->rt);
	}

	free(f);
}

// Frees an AST's memory, including its childs.
void ASTnode_free(struct ASTnode *x) {
	if (x == NULL) {
		return;
	}

	switch (x->op) {
		case A_IF: {
			ASTnode_free(((struct ASTifnode*)x)->cond);
		}	// fall through

		case A_ASSIGN:
		case A_ADD: case A_SUB: case A_MUL: case A_DIV:
		case A_EQ: case A_NE: case A_GT: case A_LT: case A_GE: case A_LE:
		case A_WHILE: {
			struct ASTbinnode *t = (void*)x;
			ASTnode_free(t->left);
			ASTnode_free(t->right);
		}	break;

		case A_PRINT: case A_RETURN:
		case A_LNOT: case A_BNOT: case A_NEG: {
			struct ASTunnode *t = (void*)x;
			ASTnode_free(t->left);
		}	break;

		case A_BLOCK: {
			struct ASTblocknode *t = (void*)x;
			struct llist_node *p = t->st.head, *nxt;
			while (p) {
				nxt = p->nxt;
				ASTnode_free((void*)p);
				p = nxt;
			}
		}	break;

		case A_LIT_I32: case A_LIT_I64: {
		}	break;

		default: {
			fail_ast_op(x->op, __FUNCTION__);
		}	break;
	}
	free(x);
}
