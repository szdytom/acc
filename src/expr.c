#include <stdio.h>
#include <stdlib.h>
#include "scan.h"
#include "scan.h"
#include "defs.h"
#include "ast.h"

static struct token Token; // current token for parsing

// Check that we have a binary operator and return its precedence.
static int op_precedence(int t) {
	switch (t) {
		case T_PLUS:
		case T_MINUS:
			return (10);
		case T_STAR:
		case T_SLASH:
			return (20);
		default:
			fprintf(stderr, "syntax error on line %d: operator expeted, got %d.\n", Line, t);
			exit(1);
	}
}

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

static void next(void) {
	scan(&Token);
}

// Parse a primary factor and return an
// AST node representing it.
static struct ASTnode* primary(void) {
	struct ASTnode *res;

	// For an INTLIT token, make a leaf AST node for it
	// and scan in the next token. Otherwise, a syntax error
	// for any other token type.
	if (Token.type == T_INTLIT) {
		res = make_leaf(A_INTLIT, Token.intval);
		next();
	} else {
		fprintf(stderr, "syntax error on line %d: primary expression excpeted.\n", Line);
		exit(1);
	}
	return (res);
}

// Return an AST tree whose root is a binary operator
static struct ASTnode* binexpr(int precedence) {
	struct ASTnode *left, *right;

	left = primary();
	int tt = Token.type;
	if (tt == T_EOF) {
		return (left);
	}

	int tp = op_precedence(tt);
	while (tp > precedence) {
		next();
		right = binexpr(tp);
		left = make_astnode(arithop(tt), left, right, 0); // join right into left

		tt = Token.type;
		if (tt == T_EOF) {
			return (left);
		}
		tp = op_precedence(tt);
	}
	return (left);
}

// Parse ans return the full ast
struct ASTnode* parse(void) {
	next();
	return (binexpr(0));
}
