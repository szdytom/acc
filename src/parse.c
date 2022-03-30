#include <stdio.h>
#include <stdlib.h>
#include "scan.h"
#include "token.h"
#include "ast.h"
#include "symbol.h"

static struct token Token;	// current token for parsing
static int skip_semi = 0;	// can skip statement semi (after block)

// Check that we have a binary operator and return its precedence.
// operators with larger precedence value will be evaluated first
static int op_precedence(int t) {
	switch (t) {
		case T_ASSIGN:
			return (20);
		case T_GT: case T_GE: case T_LT: case T_LE:
			return (40);
		case T_EQ: case T_NE:
			return (50);
		case T_PLUS: case T_MINUS:
			return (70);
		case T_STAR: case T_SLASH:
			return (90);
		default:
			fprintf(stderr, "syntax error on line %d: expected an operator, got %s.\n", Line, token_typename[t]);
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
		{T_ASSIGN,	A_ASSIGN},
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

// operator ssociativity direction
// Return	0 if left to right, e.g. +
// 		1 if right to left, e.g. =
static int direction_rtl(int t) {
	switch(t) {
		case T_ASSIGN:
			return (1);
		default:
			return (0);
	}
}

// Next token
static void next(void) {
	scan(&Token);
}

// match a token or report syntax error
static void match(int t) {
	if (t == T_SEMI && skip_semi) {
		skip_semi = 0;
	} else if (Token.type == t) {
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

static struct ASTnode* statement(void);
static struct ASTnode* expression(void);

// Parse a primary factor and return an
// AST node representing it.
static struct ASTnode* primary(void) {
	struct ASTnode *res;

	if (Token.type == T_LP) {
		// ( expr ) considered as primary
		next();
		res = expression();
		match(T_RP);
	} else if (Token.type == T_INTLIT) {
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

// Check if it is binary operator
static int is_binop(int t) {
	return (T_ASSIGN <= t && t <= T_GE);
}

// Return an AST tree whose root is a binary operator
static struct ASTnode* binexpr(int precedence) {
	struct ASTnode *left, *right;

	left = primary();
	int tt = Token.type;
	if (!is_binop(tt)) {
		return (left);
	}

	int tp = op_precedence(tt);
	while (tp > precedence) {
		next();

		if (direction_rtl(tt)) {
			right = binexpr(precedence);
			left = ast_make_assign(arithop(tt), ((struct ASTvarnode*)left)->id, right);
		} else {
			right = binexpr(tp);
			left = ast_make_binary(arithop(tt), left, right); // join right into left
		}

		tt = Token.type;
		if (!is_binop(tt)) {
			return (left);
		}
		tp = op_precedence(tt);
	}
	return (left);
}

// parse one block of code, e.g. { a; b; }
static struct ASTnode* parse_block(void) {
	struct ASTblocknode* res = (struct ASTblocknode*)ast_make_block();

	match(T_LB);
	while (Token.type != T_RB) {
		struct ASTnode *x;
		x = statement();
		if (x) {
			llist_pushback(&res->st, x);
		}

		if (Token.type == T_EOF) {
			break;
		}
	}
	match(T_RB);
	skip_semi = 1;
	return ((struct ASTnode*)res);
}

// parse an expression
static struct ASTnode* expression(void) {
	if (Token.type == T_LB) {
		return (parse_block());
	}
	return (binexpr(0));
}

// parse one print statement
static struct ASTnode* print_statement(void) {
	match(T_PRINT);
	struct ASTnode *res = ast_make_unary(A_PRINT, expression());
	match(T_SEMI);
	return (res);
}

// parse variable declaration statement
static struct ASTnode* var_declaration(void) {
	match(T_INT);
	check(T_INDENT);
	if (findglob(Text) != -1) {
		fprintf(stderr, "syntax error on line %d: variable redeclaration.\n", Line);
		exit(1);
	}
	addglob(Text);
	next();
	match(T_SEMI);
	return (NULL);
}

// parse an if statement
static struct ASTnode* if_statement(void) {
	match(T_IF); // if
	match(T_LP); // (
	struct ASTnode* cond = expression();
	match(T_RP); // )
	struct ASTnode* then = statement();
	struct ASTnode* else_then;
	if (Token.type == T_ELSE) {
		next(); // else
		else_then = statement();
	} else {
		else_then = ast_make_block(); // empty block
	}
	return (ast_make_if(then, else_then, cond));
}

// parse an while statement
static struct ASTnode* while_statement(void) {
	match(T_WHILE);
	match(T_LP);
	struct ASTnode* cond = expression();
	match(T_RP);
	struct ASTnode* body = statement();
	return (ast_make_binary(A_WHILE, cond, body));
}

// parse one statement
static struct ASTnode* statement(void) {
	if (Token.type == T_SEMI) {
		return ast_make_intlit(1);
	} else if (Token.type == T_PRINT) {
		return (print_statement());
	} else if (Token.type == T_INT) {
		return (var_declaration());
	} else if (Token.type == T_IF) {
		return (if_statement());
	} else if (Token.type == T_WHILE) {
		return (while_statement());
	} else {
		skip_semi = 0;
		struct ASTnode *res = expression();
		match(T_SEMI);
		return (res);
	}
}

// Parse ans return the full ast
struct ASTnode* parse(void) {
	next();
	return (statement());
}
