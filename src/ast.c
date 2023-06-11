#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "fatals.h"
#include "util/linklist.h"

// Build and return a binary AST node
struct ASTnode* ast_make_binary(int op, struct ASTnode *left, struct ASTnode *right) {
	struct ASTbinnode *x = malloc_or_fail(sizeof(struct ASTbinnode), __FUNCTION__);

	x->op = op;
	x->left = left;
	x->right = right;
	return ((struct ASTnode*)x);
}

// Make an AST int32 literal node
struct ASTnode* ast_make_lit_i32(int32_t v) {
	struct ASTlitnode *x = malloc_or_fail(sizeof(struct ASTlitnode), __FUNCTION__);

	x->op = A_LIT;
	x->type.vt = V_I32;
	x->val = malloc_or_fail(sizeof(int32_t), __FUNCTION__);
	*(int64_t*)x->val = v;
	return ((struct ASTnode*)x);
}

// Make an AST int32 literal node
struct ASTnode* ast_make_lit_i64(int64_t v) {
	struct ASTlitnode *x = malloc_or_fail(sizeof(struct ASTlitnode), __FUNCTION__);

	x->op = A_LIT;
	x->type.vt = V_I64;
	x->val = malloc_or_fail(sizeof(int64_t), __FUNCTION__);
	*(int64_t*)x->val = v;
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
struct ASTnode* ast_make_unary(int op, struct ASTnode *c) {
	struct ASTunnode *x = malloc_or_fail(sizeof(struct ASTunnode), __FUNCTION__);

	x->op = op;
	x->c = c;
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
struct ASTnode* ast_make_assign(int op, int left, struct ASTnode *right) {
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

// Translate ast operation type to ast node type
int ast_type(int t) {
	switch (t) {
	case A_ADD: case A_SUB: case A_MUL: case A_DIV:
	case A_EQ: case A_NE: case A_GT: case A_LT: case A_GE: case A_LE:
	case A_IF: case A_WHILE:
		return (N_BIN);
	case A_ASSIGN:
		return (N_ASSIGN);
	case A_LIT: case A_VAR:
		return (N_LEAF);
	case A_BLOCK:
		return (N_MULTI);
	case A_PRINT:
		return (N_UN);
	default:
		fail_ast_op(t, __FUNCTION__);
	}
}

// free an AST's memory
void ast_free(struct ASTnode *x) {
	if (x == NULL) {
		return;
	}

	switch (ast_type(x->op)) {
	case N_ASSIGN: {
		struct ASTassignnode *t = (struct ASTassignnode*)x;
		ast_free(t->right);
	}	break;
	case N_BIN: {
		struct ASTbinnode *t = (struct ASTbinnode*)x;
		ast_free(t->left);
		ast_free(t->right);
		if (x->op == A_IF) {
			ast_free(((struct ASTifnode*)x)->cond);
		}
	}	break;
	case N_UN: {
		struct ASTunnode *t = (struct ASTunnode*)x;
		ast_free(t->c);
	}	break;
	case N_MULTI: {
		struct ASTblocknode *t = (struct ASTblocknode*)x;
		struct llist_node *p = t->st.head;
		while (p) {
			ast_free(p->val);
			p = p->nxt;
		}
		llist_free(&t->st);
	}	break;
	case N_LEAF: {
		struct ASTlitnode *t = (struct ASTlitnode*)x;
		if (t->op == A_LIT) {
			if (t->val) {
				free(t->val);
			}
		}
	}	break;
	}
	free(x);
}
