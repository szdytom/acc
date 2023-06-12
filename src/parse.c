#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "scan.h"
#include "token.h"
#include "ast.h"
#include "fatals.h"

static struct linklist Tokens;	// current token for parsing

// Check that we have a binary operator and return its precedence.
// operators with larger precedence value will be evaluated first
static int op_precedence(struct token *t) {
	switch (t->type) {
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
			fail_ce_expect(t->line, "an operator", token_typename[t->type]);
	}
}

// Convert a arithmetic token into an AST operation.
static int arithop(struct token *t) {
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
		{-1}
	};

	for (int i = 0; map[i][0] != -1; ++i) {
		if (t->type == map[i][0]) {
			return map[i][1];
		}
	}
	fail_ce_expect(t->line, "an binary operator", token_typename[t->type]);
}

// operator ssociativity direction
// Returns	false if left to right, e.g. +
// 		true if right to left, e.g. =
static bool direction_rtl(int t) {
	switch(t) {
		case T_ASSIGN:
			return (true);
		default:
			return (false);
	}
}

// Next token
static void next(void) {
	if (Tokens.head) {
		token_free(llist_popfront(&Tokens));
	}
}

// preview next kth token from input stream
static struct token* preview(int k) {
	if (Tokens.length <= k) {
		static struct token token_eof = {
			.type = T_EOF
		};
		return (&token_eof);
	}

	struct token* res = llist_get(&Tokens, k);
	return (res);
}

// return current token from input stream
static struct token* current(void) {
	return (preview(0));
}

// match a token or report syntax error
static void match(int t) {
	if (current()->type == t) {
		next();
	} else {
		fail_ce_expect(current()->line, token_typename[t], token_typename[current()->type]);
	}
}

// check current token's type or report syntax error.
static void expect(int t) {
	if (current()->type != t) {
		fail_ce_expect(current()->line, token_typename[t], token_typename[current()->type]);
	}
}

static struct ASTnode* statement(void);
static struct ASTnode* expression(void);

// Parse a primary factor and return an
// AST node representing it.
static struct ASTnode* primary(void) {
	struct ASTnode *res;
	struct token *t = current();
	if (t->type == T_LP) {
		// ( expr ) considered as primary
		next();
		res = expression();
		match(T_RP);
	} else if (t->type == T_I32_LIT) {
		res = ast_make_lit_i32(t->val_i32);
		next();
	} else if (t->type == T_I64_LIT) {
		res = ast_make_lit_i64(current()->val_i64);
		next();
	} else if (t->type == T_ID) {
		// TODO: identifier.
		fail_ce(t->line, "got an identifier");
		/*
		int id = findglob((char*)current()->val);
		if (id == -1) {
			fprintf(stderr, "syntax error on line %d: unknown indentifier %s.\n", Line, (char*)current()->val);
			exit(1);
		}
		next();
		return (ast_make_var(id));
		*/
	} else {
		fail_ce(t->line, "primary expression expected");
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
	struct token *op = current();
	if (!is_binop(op->type)) {
		return (left);
	}

	int tp = op_precedence(op);
	while (tp > precedence) {
		next();

		if (direction_rtl(op->type)) {
			right = binexpr(precedence);
			left = ast_make_assign(arithop(op), left, right);
		} else {
			right = binexpr(tp);
			left = ast_make_binary(arithop(op), left, right); // join right into left
		}

		op = current();
		if (!is_binop(op->type)) {
			return (left);
		}
		tp = op_precedence(op);
	}
	return (left);
}

// parse one block of code, e.g. { a; b; }
static struct ASTnode* block(void) {
	match(T_LB);
	if (current()->type == T_RB) {
		next();
		return NULL;
	}

	struct ASTblocknode* res = (struct ASTblocknode*)ast_make_block();
	while (current()->type != T_RB) {
		struct ASTnode *x;
		x = statement();
		llist_pushback_notnull(&res->st, x);

		if (current()->type == T_EOF) {
			break;
		}
	}
	match(T_RB);
	return ((struct ASTnode*)res);
}

// parse an expression
static struct ASTnode* expression(void) {
	if (current()->type == T_SEMI) {
		return (NULL);
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

/*
// parse variable declaration statement
static struct ASTnode* var_declaration(void) {
	match(T_INT);
	expect(T_IDENT);
	if (findglob((char*)current()->val) != -1) {
		fail_ce("variable declared twice.");
	}
	addglob((char*)current()->val);
	next();
	match(T_SEMI);
	return (NULL);
}
*/

// parse an if statement
static struct ASTnode* if_statement(void) {
	match(T_IF); // if
	match(T_LP); // (
	struct ASTnode* cond = expression();
	match(T_RP); // )
	struct ASTnode* then = statement();
	struct ASTnode* else_then;
	if (current()->type == T_ELSE) {
		next(); // else
		else_then = statement();
	} else {
		else_then = NULL; // empty block
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

// parse a for statement (into a while loop)
static struct ASTnode* for_statement(void) {
	match(T_FOR);
	match(T_LP);
	struct ASTnode *init = statement();

	struct ASTnode *cond;
	if (current()->type != T_SEMI) {
		cond = expression();
	} else {
		cond = ast_make_lit_i32(1);
	}
	next(); // skip the ;

	struct ASTnode *inc;
	if (current()->type != T_RP) {
		inc = expression();
	} else {
		inc = NULL;
	}

	match(T_RP);
	struct ASTnode *body = statement();
	struct ASTblocknode *container = (struct ASTblocknode*)ast_make_block();
	struct ASTnode *wbody;

	if (body == NULL && inc == NULL) {
		wbody = NULL;
	} else if (body == NULL) {
		wbody = inc;
	} else if (inc == NULL) {
		wbody = body;
	} else {
		struct ASTblocknode* wt = (struct ASTblocknode*)ast_make_block();
		llist_pushback_notnull(&wt->st, body);
		llist_pushback_notnull(&wt->st, inc);
		wbody = (struct ASTnode*)wt;
	}

	llist_pushback_notnull(&container->st, init);
	llist_pushback(&container->st, ast_make_binary(A_WHILE, cond, wbody));
	return ((struct ASTnode*)container);
}

static struct ASTnode* return_statement() {
	match(T_RETURN);
	struct ASTnode *res = expression();
	match(T_SEMI);
	return (ast_make_unary(A_RETURN, res));
}

// parse one statement
static struct ASTnode* statement(void) {
	switch (current()->type) {
		case T_LB:
			return (block());

		case T_SEMI:
			return (NULL);

		case T_PRINT:
			return (print_statement());

//		case T_INT:
//			return (var_declaration());
		case T_IF:
			return (if_statement());

		case T_WHILE:
			return (while_statement());

		case T_FOR:
			return (for_statement());

		case T_RETURN:
			return (return_statement());

		default: {
			struct ASTnode* res = expression();
			match(T_SEMI);
			return (res);
		}
	}
}

// Parse one top-level function
// Sets the func_name param.
static struct ASTnode* function(char **func_name) {
	match(T_INT);
	expect(T_ID);
	*func_name = current()->val_s;	// transfer ownership of the identifier string to caller
	current()->val_s = NULL;	// prevent it from being freed in token_free() called by next.
	next();

	match(T_LP);
	if (current()->type == T_VOID) {
		next();
		goto END_PARAM_LIST;
	}
	// TODO: param list

END_PARAM_LIST:
	match(T_RP);
	return (block());
}

// Parse ans return the full ast
struct ASTnode* parse(const char *name) {
	Tokens = scan_tokens(name);
	char *func_name;
	struct ASTnode* res = function(&func_name);

	free(func_name);
	while (Tokens.length > 0) {
		token_free(llist_popfront(&Tokens));
	}
	llist_free(&Tokens);
	return (res);
}
