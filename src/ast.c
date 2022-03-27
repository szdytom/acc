#include <stdlib.h>
#include <stdio.h>
#include "defs.h"

// Build and return a generic AST node
struct ASTnode* make_astnode(int op, struct ASTnode *left, struct ASTnode *right, int intval) {
	struct ASTnode *x = malloc(sizeof(struct ASTnode));
	if (x == NULL) {
		// malloc failure
		fprintf(stderr, "Unable to malloc in make_astnode()\n");
		exit(1);
	}

	x->op = op;
	x->left = left;
	x->right = right;
	x-> intval = intval;
	return (x);
}

// Make an AST leaf node
struct ASTnode* make_leaf(int op, int intval) {
	return (make_astnode(op, NULL, NULL, intval));
}

// Make a unary AST node: only one child
struct ASTnode* make_unary(int op, struct ASTnode *c, int intval) {
	return (make_astnode(op, c, NULL, intval));
}

