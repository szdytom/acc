#include <stdlib.h>
#include <stdio.h>
#include "defs.h"
#include "util/linklist.h"

// Build and return a binary AST node
struct ASTnode* ast_make_binary(int op, struct ASTnode *left, struct ASTnode *right) {
	struct ASTbinnode *x = malloc(sizeof(struct ASTbinnode));
	if (x == NULL) {
		// malloc failure
		fprintf(stderr, "Unable to malloc in %s.\n", __FUNCTION__);
		exit(1);
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
		fprintf(stderr, "Unable to malloc in %s.\n", __FUNCTION__);
		exit(1);
	}

	x->op = A_INTLIT;
	x->val = val;
	return ((struct ASTnode*)x);
}

// Make a unary AST node: only one child
struct ASTnode* ast_make_unary(int op, struct ASTnode *c) {
	struct ASTunnode *x = malloc(sizeof(struct ASTunnode));
	if (x == NULL) {
		fprintf(stderr, "Unable to malloc in %s.\n", __FUNCTION__);
		exit(1);
	}

	x->op = op;
	x->c = c;
	return ((struct ASTnode*)x);
}

// Make a block ast node
struct ASTnode* ast_make_block() {
	struct ASTblocknode *x = malloc(sizeof(struct ASTblocknode));
	if (x == NULL) {
		fprintf(stderr, "Unable to malloc in %s.\n", __FUNCTION__);
		exit(1);
	}

	x->op = A_BLOCK;
	llist_init(&x->st);
	return ((struct ASTnode*)x);
}

// translate ast operation type to ast node type
int ast_type(int t) {
	if (t == A_ADD || t == A_SUB || t == A_MUL || t == A_DIV) {
		return (N_BIN);
	}
	if (t == A_INTLIT) {
		return (N_LEAF);
	}
	if (t == A_BLOCK) {
		return (N_MULTI);
	}
	if (t == A_PRINT) {
		return (N_UN);
	}
	fprintf(stderr, "Unknown operation type %d.\n", t);
	exit(1);
}

// free an AST's memory
void free_ast(struct ASTnode *x) {
	if (x == NULL) {
		return;
	}

	int nt = ast_type(x->op);
	if (nt == N_BIN) {
		struct ASTbinnode *t = (struct ASTbinnode*)x;
		free_ast(t->left);
		free_ast(t->right);
	} else if (nt == N_UN) {
		struct ASTunnode *t = (struct ASTunnode*)x;
		free_ast(t->c);
	} else if (nt == N_MULTI) {
		struct ASTblocknode *t = (struct ASTblocknode*)x;
		struct llist_node *p = t->st.head;
		while (p) {
			free_ast(p->val);
			p = p->nxt;
		}
		llist_free(&t->st);
	}
	free(x);
}
