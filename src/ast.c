#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "fatals.h"
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
struct ASTnode* ast_make_binary(int op, struct ASTnode *left, struct ASTnode *right) {
	struct ASTbinnode *x = malloc_or_fail(sizeof(struct ASTbinnode), __FUNCTION__);

	x->op = op;
	x->left = left;
	x->right = right;
	return ((struct ASTnode*)x);
}

// Make an AST integer literal (32bits) node
struct ASTnode* ast_make_lit_i32(int32_t v) {
	struct ASTi32node *x = malloc_or_fail(sizeof(struct ASTi32node), __FUNCTION__);

	x->op = A_LIT_I32;
	x->val = v;
	return ((struct ASTnode*)x);
}

// Make an AST integer literal (64bits) node
struct ASTnode* ast_make_lit_i64(int64_t v) {
	struct ASTi64node *x = malloc_or_fail(sizeof(struct ASTi64node), __FUNCTION__);

	x->op = A_LIT_I64;
	x->val = v;
	return ((struct ASTnode*)x);
}

// Make an AST variable value node
struct ASTnode* ast_make_var(int id) {
	struct ASTvarnode *x = malloc_or_fail(sizeof(struct ASTvarnode), __FUNCTION__);

	x->op = A_VAR;
	x->id = id;
	return ((struct ASTnode*)x);
}

// Make a unary AST node: only one child
struct ASTnode* ast_make_unary(int op, struct ASTnode *child) {
	struct ASTunnode *x = malloc_or_fail(sizeof(struct ASTunnode), __FUNCTION__);

	x->op = op;
	x->left = child;
	return ((struct ASTnode*)x);
}

// Make a block ast node
struct ASTnode* ast_make_block() {
	struct ASTblocknode *x = malloc_or_fail(sizeof(struct ASTblocknode), __FUNCTION__);

	x->op = A_BLOCK;
	llist_init(&x->st);
	return ((struct ASTnode*)x);
}

// Make a assignment ast node
struct ASTnode* ast_make_assign(int op, struct ASTnode *left, struct ASTnode *right) {
	struct ASTassignnode *x = malloc_or_fail(sizeof(struct ASTassignnode), __FUNCTION__);

	x->op = op;
	x->left = left;
	x->right = right;
	return ((struct ASTnode*)x);
}

// Make a if statement ast node
struct ASTnode* ast_make_if(struct ASTnode *left, struct ASTnode *right, struct ASTnode *cond) {
	struct ASTifnode *x = malloc_or_fail(sizeof(struct ASTifnode), __FUNCTION__);

	x->op = A_IF;
	x->left = left;
	x->right = right;
	x->cond = cond;
	return ((struct ASTnode*)x);
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
void ast_debug_print(FILE *Outfile, struct ASTnode *rt) {
	ast_print_dfs(Outfile, rt, 0);
}

// Prints the structure of a Afunction into Outfile.
void afunc_debug_print(FILE *Outfile, struct Afunction *f) {
	fprintf(Outfile, "FUNCTION %s: \n", f->name);
	ast_print_dfs(Outfile, f->rt, 0);
}

// Constructs a Afunction.
struct Afunction* afunc_make() {
	struct Afunction *res = (struct Afunction*)malloc_or_fail(sizeof(struct Afunction), __FUNCTION__);

	res->rt = NULL;
	res->name = NULL;
	return res;
}

// Frees a Afunction and all its components.
void afunc_free(struct Afunction *f) {
	if (f->name) {
		free(f->name);
	}

	if (f->rt) {
		ast_free(f->rt);
	}

	free(f);
}

// Frees an AST's memory, including its childs.
void ast_free(struct ASTnode *x) {
	if (x == NULL) {
		return;
	}

	switch (x->op) {
		case A_IF: {
			ast_free(((struct ASTifnode*)x)->cond);
		}	[[fallthrough]];

		case A_ASSIGN:
		case A_ADD: case A_SUB: case A_MUL: case A_DIV:
		case A_EQ: case A_NE: case A_GT: case A_LT: case A_GE: case A_LE:
		case A_WHILE: {
			struct ASTbinnode *t = (struct ASTbinnode*)x;
			ast_free(t->left);
			ast_free(t->right);
		}	break;

		case A_PRINT: case A_RETURN:
		case A_LNOT: case A_BNOT: case A_NEG: {
			struct ASTunnode *t = (struct ASTunnode*)x;
			ast_free(t->left);
		}	break;

		case A_BLOCK: {
			struct ASTblocknode *t = (struct ASTblocknode*)x;
			struct llist_node *p = t->st.head, *nxt;
			while (p) {
				nxt = p->nxt;
				ast_free((struct ASTnode*)p);
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
