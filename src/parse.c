#include <stdio.h>
#include <stdlib.h>
#include "scan.h"
#include "defs.h"
#include "ast.h"
#include "symbol.h"

static struct token Token; // current token for parsing

// Check that we have a binary operator and return its precedence.
static int op_precedence(int t) {
	switch (t) {
		case T_GT: case T_GE: case T_LT: case T_LE:
			return (10);
		case T_EQ: case T_NE:
			return (20);
		case T_PLUS: case T_MINUS:
			return (40);
		case T_STAR: case T_SLASH:
			return (80);
		default:
			fprintf(stderr, "syntax error on line %d: operator expeted, got %s.\n", Line, token_typename[t]);
			exit(1);
	}
}

// Convert a arithmetic token into an AST operation.
static int arithop(int t) {
	static const int map[][2] = {
		{T_PLUS,	A_ADD},
		{T_MINUS,	A_SUB},
		{T_STAR,	A_MUL},
		{T_SLASH,	A_DIV},
		{T_EQ,		A_EQ},
		{T_NE,		A_NE},
		{T_LT,		A_LT},
		{T_LE,		A_LE},
		{T_GT,		A_GT},
		{T_GE,		A_GE},
		{T_EOF}
	};

	for (int i = 0; map[i][0] != T_EOF; ++i) {
		if (t == map[i][0]) {
			return map[i][1];
		}
	}
	fprintf(stderr, "syntax error on line %d: expected an operator, got %s.\n", Line, token_typename[t]);
	exit(1);
}

static void next(void) {
	scan(&Token);
}

// Parse a primary factor and return an
// AST node representing it.
static struct ASTnode* primary(void) {
	struct ASTnode *res;

	if (Token.type == T_INTLIT) {
		res = ast_make_intlit(Token.intval);
		next();
	} else if (Token.type == T_INDENT) {
		int id = findglob(Text);
		if (id == -1) {
			fprintf(stderr, "syntax error on line %d: unknown indentifier %s.\n", Line, Text);
			exit(1);
		}
		next();
		return (ast_make_var(id));
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
	if (tt == T_SEMI) {
		return (left);
	}

	int tp = op_precedence(tt);
	while (tp > precedence) {
		next();
		right = binexpr(tp);
		left = ast_make_binary(arithop(tt), left, right); // join right into left

		tt = Token.type;
		if (tt == T_SEMI) {
			return (left);
		}
		tp = op_precedence(tt);
	}
	return (left);
}

// match a token or report syntax error
static void match(int t) {
	if (Token.type == t) {
		next();
	} else {
		fprintf(stderr, "syntax error on line %d: %s excpected, got %s.\n"
				, Line, token_typename[t], token_typename[Token.type]);
		exit(1);
	}
}

// check current token's type or report syntax error.
static void check(int t) {
	if (Token.type != t) {
		fprintf(stderr, "syntax error on line %d: %s excpected, got %s.\n"
				, Line, token_typename[t], token_typename[Token.type]);
		exit(1);
	}
}

// parse an expression
static struct ASTnode* expression(void) {
	return binexpr(0);
}

// parse one print statement
static struct ASTnode* print_statement(void) {
	match(T_PRINT);
	struct ASTnode *res = ast_make_unary(A_PRINT, expression());
	return res;
}

// parse variable declaration statement
static void var_declaration(void) {
	match(T_INT);
	check(T_INDENT);
	if (findglob(Text) != -1) {
		fprintf(stderr, "syntax error on line %d: variable redeclaration.\n", Line);
		exit(1);
	}
	addglob(Text);
	next();
}

// parse value assignment statement
static struct ASTnode* assign_statement(void) {
	check(T_INDENT);

	int left = findglob(Text);
	if (left == -1) {
		fprintf(stderr, "syntax error on line %d: unknown indentifier %s.\n", Line, Text);
		exit(1);
	}

	next();
	match(T_ASSIGN);
	return ast_make_assign(A_ASSIGN, left, expression());
}

// parse one or multiple statements
static struct ASTnode* statements(void) {
	struct ASTblocknode *res = (struct ASTblocknode*)ast_make_block();
	while (1) {
		struct ASTnode *s = NULL;
		if (Token.type == T_PRINT) {
			s = print_statement();
		} else if (Token.type == T_INT) {
			var_declaration();
		} else if (Token.type == T_INDENT) {
			s = assign_statement();
		} else {
			fprintf(stderr, "syntax error on line %d: statement expected.\n", Line);
			exit(1);
		}

		match(T_SEMI);
		if (s) {
			llist_pushback(&res->st, s);
		}

		if (Token.type == T_EOF) {
			return ((struct ASTnode*)res);
		}
	}
}

// Parse ans return the full ast
struct ASTnode* parse(void) {
	next();
	return (statements());
}
