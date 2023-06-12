#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "fatals.h"
#include "util/linklist.h"

const char *ast_opname[] = {
	"=",
	"+", "-", "*", "/",
	"==", "!=", "<", ">", "<=", ">=",
	"int32", "int64",
	"var",
	"block",
	"print", "if", "while",
	"return",
	NULL
};

// Build and return a binary AST node
struct ASTnode* ast_make_binary(int op, struct ASTnode *left, struct ASTnode *right) {
	struct ASTbinnode *x = malloc_or_fail(sizeof(struct ASTbinnode), __FUNCTION__);

	x->op = op;
	x->left = left;
	x->right = right;
	return ((struct ASTnode*)x);
}

// Make an AST integer literal (32bit) node
struct ASTnode* ast_make_lit_i32(int32_t v) {
	struct ASTi32node *x = malloc_or_fail(sizeof(struct ASTi32node), __FUNCTION__);

	x->op = A_LIT_I32;
	x->val = v;
	return ((struct ASTnode*)x);
}

// Make an AST integer literal (64bit) node
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

// free an AST's memory
void ast_free(struct ASTnode *x) {
	if (x == NULL) {
		return;
	}

	switch (x->op) {
		case A_IF: {
			ast_free(((struct ASTifnode*)x)->cond);
		}	// dont break

		case A_ASSIGN:
		case A_ADD: case A_SUB: case A_MUL: case A_DIV:
		case A_EQ: case A_NE: case A_GT: case A_LT: case A_GE: case A_LE:
		case A_WHILE: {
			struct ASTbinnode *t = (struct ASTbinnode*)x;
			ast_free(t->left);
			ast_free(t->right);
		}	break;
	
		case A_PRINT: case A_RETURN: {
			struct ASTunnode *t = (struct ASTunnode*)x;
			ast_free(t->left);
		}	break;

		case A_BLOCK: {
			struct ASTblocknode *t = (struct ASTblocknode*)x;
			struct llist_node *p = t->st.head, *nxt;
			while (p) {
				nxt = p->nxt;
				ast_free(p);
				p = nxt;
			}
			llist_free(&t->st);
		}	break;

		default: {
		}	break;
	}
	free(x);
}
