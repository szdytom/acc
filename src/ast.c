#include <stdlib.h>
#include <stdio.h>
#include "ast.h"
#include "fatals.h"
#include "util/linklist.h"

// Build and return a binary AST node
struct ASTnode* ast_make_binary(int op, struct ASTnode *left, struct ASTnode *right) {
	struct ASTbinnode *x = malloc(sizeof(struct ASTbinnode));
	if (x == NULL) {
		fail_malloc(__FUNCTION__);
	}

	x->op = op;
	x->left = left;
	x->right = right;
	return ((struct ASTnode*)x);
}

// Make an AST int literal node
struct ASTnode* ast_make_intlit(int val) {
	struct ASTintnode *x = malloc(sizeof(struct ASTintnode));
	if (x == NULL) {
		fail_malloc(__FUNCTION__);
	}

	x->op = A_INTLIT;
	x->val = val;
	return ((struct ASTnode*)x);
}

// Make an AST variable value node
struct ASTnode* ast_make_var(int id) {
	struct ASTvarnode *x = malloc(sizeof(struct ASTvarnode));
	if (x == NULL) {
		fail_malloc(__FUNCTION__);
	}

	x->op = A_VAR;
	x->id = id;
	return ((struct ASTnode*)x);
}

// Make a unary AST node: only one child
struct ASTnode* ast_make_unary(int op, struct ASTnode *c) {
	struct ASTunnode *x = malloc(sizeof(struct ASTunnode));
	if (x == NULL) {
		fail_malloc(__FUNCTION__);
	}

	x->op = op;
	x->c = c;
	return ((struct ASTnode*)x);
}

// Make a block ast node
struct ASTnode* ast_make_block() {
	struct ASTblocknode *x = malloc(sizeof(struct ASTblocknode));
	if (x == NULL) {
		fail_malloc(__FUNCTION__);
	}

	x->op = A_BLOCK;
	llist_init(&x->st);
	return ((struct ASTnode*)x);
}

// Make a assignment ast node
struct ASTnode* ast_make_assign(int op, int left, struct ASTnode *right) {
	struct ASTassignnode *x = malloc(sizeof(struct ASTassignnode));
	if (x == NULL) {
		fail_malloc(__FUNCTION__);
	}

	x->op = op;
	x->left = left;
	x->right = right;
	return ((struct ASTnode*)x);
}

// Make a if statement ast node
struct ASTnode* ast_make_if(struct ASTnode *left, struct ASTnode *right, struct ASTnode *cond) {
	struct ASTifnode *x = malloc(sizeof(struct ASTifnode));
	if (x == NULL) {
		fail_malloc(__FUNCTION__);
	}

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
		case A_INTLIT: case A_VAR:
			return (N_LEAF);
		case A_BLOCK:
			return (N_MULTI);
		case A_PRINT:
			return (N_UN);
		default:
			fprintf(stderr, "%s: unknown operation type %d.\n", __FUNCTION__, t);
			exit(1);
	}
}

// free an AST's memory
void ast_free(struct ASTnode *x) {
	if (x == NULL) {
		return;
	}

	int nt = ast_type(x->op);
	if (nt == N_ASSIGN) {
		struct ASTassignnode *t = (struct ASTassignnode*)x;
		ast_free(t->right);
	} else if (nt == N_BIN) {
		struct ASTbinnode *t = (struct ASTbinnode*)x;
		ast_free(t->left);
		ast_free(t->right);
		if (x->op == A_IF) {
			ast_free(((struct ASTifnode*)x)->cond);
		}
	} else if (nt == N_UN) {
		struct ASTunnode *t = (struct ASTunnode*)x;
		ast_free(t->c);
	} else if (nt == N_MULTI) {
		struct ASTblocknode *t = (struct ASTblocknode*)x;
		struct llist_node *p = t->st.head;
		while (p) {
			ast_free(p->val);
			p = p->nxt;
		}
		llist_free(&t->st);
	}
	free(x);
}
