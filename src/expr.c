#include <stdio.h>
#include <stdlib.h>
#include "scan.h"
#include "scan.h"
#include "defs.h"
#include "ast.h"

static struct token Token; // current token for parsing

// Convert a arithmetic token into an AST operation.
static int arithop(int t) {
	switch (t) {
		case T_PLUS:
			return (A_ADD);
		case T_MINUS:
			return (A_SUB);
		case T_STAR:
			return (A_MUL);
		case T_SLASH:
			return (A_DIV);
		default:
			fprintf(stderr, "unknown token in arithop() on line %d\n", Line);
			exit(1);
	}
}

// Parse a primary factor and return an
// AST node representing it.
static struct ASTnode* primary(void) {
	struct ASTnode *res;

	// For an INTLIT token, make a leaf AST node for it
	// and scan in the next token. Otherwise, a syntax error
	// for any other token type.
	if (Token.token == T_INTLIT) {
		res = make_leaf(A_INTLIT, Token.intval);
		scan(&Token);
	} else {
		fprintf(stderr, "syntax error on line %d: primary expression excpeted.\n", Line);
		exit(1);
	}
	return (res);
}

// Return an AST tree whose root is a binary operator
static struct ASTnode* binexpr(void) {
	struct ASTnode *left, *right;

	left = primary();
	if (Token.token == T_EOF) {
		return (left);
	}

	int nodetype = arithop(Token.token);
	scan(&Token);
	right = binexpr();
	return (make_astnode(nodetype, left, right, 0));
}

struct ASTnode* parse(void) {
	scan(&Token);
	return (binexpr());
}
