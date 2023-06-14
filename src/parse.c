#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "scan.h"
#include "token.h"
#include "ast.h"
#include "fatals.h"

// Parsing Context
struct Pcontext {
	struct linklist tokens;	// token list
	struct llist_node *cur;	// current token
};

// Checks that we have a binary operator and return its precedence.
// Operators with larger precedence value will be evaluated first.
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

// Converts a binary arithmetic token into an AST operation.
static int binary_arithop(struct token *t) {
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
		{T_LAND,	A_LAND},
		{T_LOR,		A_LOR},
		{-1}
	};

	for (int i = 0; map[i][0] != -1; ++i) {
		if (t->type == map[i][0]) {
			return map[i][1];
		}
	}
	fail_ce_expect(t->line, "an binary operator", token_typename[t->type]);
}

// Converts a unary arithmetic token into an AST operation.
static int unary_arithop(struct token *t) {
	static const int map[][2] = {
		{T_MINUS,	A_NEG},
		{T_LNOT,	A_LNOT},
		{T_BNOT,	A_BNOT},
		{-1}
	};

	for (int i = 0; map[i][0] != -1; ++i) {
		if (t->type == map[i][0]) {
			return map[i][1];
		}
	}

	fail_ce_expect(t->line, "an unary operator", token_typename[t->type]);
}

// Operator associativity direction
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
static void next(struct Pcontext *ctx) {
	if (ctx->cur) {
		ctx->cur = ctx->cur->nxt;
	}
}

// return current token from input stream
static struct token* current(struct Pcontext *ctx) {
	static struct token token_eof = {
		.type = T_EOF
	};

	if (ctx->cur) {
		return ((void*)ctx->cur);
	}
	return (&token_eof);
}

// match a token or report syntax error
static void match(struct Pcontext *ctx, int t) {
	if (current(ctx)->type == t) {
		next(ctx);
	} else {
		fail_ce_expect(current(ctx)->line, token_typename[t], token_typename[current(ctx)->type]);
	}
}

// check current token's type or report syntax error.
static void expect(struct Pcontext *ctx, int t) {
	if (current(ctx)->type != t) {
		fail_ce_expect(current(ctx)->line, token_typename[t], token_typename[current(ctx)->type]);
	}
}

static struct ASTnode* statement(struct Pcontext *ctx);
static struct ASTnode* expression(struct Pcontext *ctx);

// Parse a primary factor and return an
// AST node representing it.
static struct ASTnode* primary(struct Pcontext *ctx) {
	struct ASTnode *res;
	struct token *t = current(ctx);

	if (t->type == T_LP) {
		// ( expr ) considered as primary
		next(ctx);
		res = expression(ctx);
		match(ctx, T_RP);
	} else if (t->type == T_I32_LIT) {
		res = ast_make_lit_i32(t->val_i32);
		next(ctx);
	} else if (t->type == T_I64_LIT) {
		res = ast_make_lit_i64(current(ctx)->val_i64);
		next(ctx);
	} else if (t->type == T_ID) {
		// TODO: identifier.
		fail_ce(t->line, "got an identifier");
		/*
		int id = findglob((char*)current(ctx)->val);
		if (id == -1) {
			fprintf(stderr, "syntax error on line %d: unknown indentifier %s.\n", Line, (char*)current(ctx)->val);
			exit(1);
		}
		next(ctx);
		return (ast_make_var(id));
		*/
	} else {
		fail_ce(t->line, "primary expression expected");
	}
	return (res);
}

// Returns whether the given token type can be a prefix operator (negation, logical not, bitwise not)
static bool is_prefix_op(int op) {
	switch (op) {
		case T_MINUS: case T_LNOT: case T_BNOT:
			return (true);

		default:
			return (false);
	}
}

// Parses a primary expression with prefixes, e.g. ~10
static struct ASTnode* prefixed_primary(struct Pcontext *ctx) {
	struct token *t = current(ctx);

	if (is_prefix_op(t->type)) {
		next(ctx);
		struct ASTnode *child = prefixed_primary(ctx);
		return (ast_make_unary(unary_arithop(t), child));
	}

	return (primary(ctx));
}

// Returns whether the given token type can be a binary operator.
static bool is_binop(int t) {
	switch (t) {
		case T_ASSIGN:
		case T_PLUS: case T_MINUS: case T_STAR: case T_SLASH:
		case T_LAND: case T_LOR:
		case T_EQ: case T_NE: case T_LT:
		case T_GT: case T_LE: case T_GE:
			return (true);

		default:
			return (false);
	}
}

// Return an AST tree whose root is a binary operator
static struct ASTnode* binexpr(struct Pcontext *ctx, int precedence) {
	struct ASTnode *left, *right;

	left = prefixed_primary(ctx);
	struct token *op = current(ctx);
	if (!is_binop(op->type)) {
		return (left);
	}

	int tp = op_precedence(op);
	while (tp > precedence) {
		next(ctx);

		if (direction_rtl(op->type)) {
			right = binexpr(ctx, precedence);
			left = ast_make_assign(binary_arithop(op), left, right);
		} else {
			right = binexpr(ctx, tp);
			left = ast_make_binary(binary_arithop(op), left, right); // join right into left
		}

		op = current(ctx);
		if (!is_binop(op->type)) {
			return (left);
		}
		tp = op_precedence(op);
	}
	return (left);
}

// parse one block of code, e.g. { a; b; }
static struct ASTnode* block(struct Pcontext *ctx) {
	match(ctx, T_LB);
	if (current(ctx)->type == T_RB) {
		next(ctx);
		return (NULL);
	}

	struct ASTblocknode* res = (struct ASTblocknode*)ast_make_block();
	while (current(ctx)->type != T_RB) {
		struct ASTnode *x;
		x = statement(ctx);
		llist_pushback_notnull(&res->st, x);

		if (current(ctx)->type == T_EOF) {
			break;
		}
	}
	match(ctx, T_RB);
	return ((struct ASTnode*)res);
}

// parse an expression
static struct ASTnode* expression(struct Pcontext *ctx) {
	if (current(ctx)->type == T_SEMI) {
		return (NULL);
	}

	return (binexpr(ctx, 0));
}

// parse one print statement
static struct ASTnode* print_statement(struct Pcontext *ctx) {
	match(ctx, T_PRINT);
	struct ASTnode *res = ast_make_unary(A_PRINT, expression(ctx));
	match(ctx, T_SEMI);
	return (res);
}

/*
// parse variable declaration statement
static struct ASTnode* var_declaration(void) {
	match(ctx, T_INT);
	expect(ctx, T_IDENT);
	if (findglob((char*)current(ctx)->val) != -1) {
		fail_ce("variable declared twice.");
	}
	addglob((char*)current(ctx)->val);
	next(ctx);
	match(ctx, T_SEMI);
	return (NULL);
}
*/

// parse an if statement
static struct ASTnode* if_statement(struct Pcontext *ctx) {
	match(ctx, T_IF); // if
	match(ctx, T_LP); // (
	struct ASTnode* cond = expression(ctx);
	match(ctx, T_RP); // )
	struct ASTnode* then = statement(ctx);
	struct ASTnode* else_then;
	if (current(ctx)->type == T_ELSE) {
		next(ctx); // else
		else_then = statement(ctx);
	} else {
		else_then = NULL; // empty block
	}
	return (ast_make_if(then, else_then, cond));
}

// parse an while statement
static struct ASTnode* while_statement(struct Pcontext *ctx) {
	match(ctx, T_WHILE);
	match(ctx, T_LP);
	struct ASTnode* cond = expression(ctx);
	match(ctx, T_RP);
	struct ASTnode* body = statement(ctx);
	return (ast_make_binary(A_WHILE, cond, body));
}

// parse a for statement (into a while loop)
static struct ASTnode* for_statement(struct Pcontext *ctx) {
	match(ctx, T_FOR);
	match(ctx, T_LP);
	struct ASTnode *init = statement(ctx);

	struct ASTnode *cond;
	if (current(ctx)->type != T_SEMI) {
		cond = expression(ctx);
	} else {
		cond = ast_make_lit_i32(1);
	}
	next(ctx); // skip the ;

	struct ASTnode *inc;
	if (current(ctx)->type != T_RP) {
		inc = expression(ctx);
	} else {
		inc = NULL;
	}

	match(ctx, T_RP);
	struct ASTnode *body = statement(ctx);
	struct ASTblocknode *container = (void*)ast_make_block();
	struct ASTnode *wbody;

	if (body == NULL && inc == NULL) {
		wbody = NULL;
	} else if (body == NULL) {
		wbody = inc;
	} else if (inc == NULL) {
		wbody = body;
	} else {
		struct ASTblocknode* wt = (void*)ast_make_block();
		llist_pushback_notnull(&wt->st, body);
		llist_pushback_notnull(&wt->st, inc);
		wbody = (void*)wt;
	}

	llist_pushback_notnull(&container->st, init);
	llist_pushback(&container->st, ast_make_binary(A_WHILE, cond, wbody));
	return ((void*)container);
}

static struct ASTnode* return_statement(struct Pcontext *ctx) {
	match(ctx, T_RETURN);
	struct ASTnode *res = expression(ctx);
	match(ctx, T_SEMI);
	return (ast_make_unary(A_RETURN, res));
}

// parse one statement
static struct ASTnode* statement(struct Pcontext *ctx) {
	switch (current(ctx)->type) {
		case T_LB:
			return (block(ctx));

		case T_SEMI:
			return (NULL);

		case T_PRINT:
			return (print_statement(ctx));

//		case T_INT:
//			return (var_declaration());
		case T_IF:
			return (if_statement(ctx));

		case T_WHILE:
			return (while_statement(ctx));

		case T_FOR:
			return (for_statement(ctx));

		case T_RETURN:
			return (return_statement(ctx));

		default: {
			struct ASTnode* res = expression(ctx);
			match(ctx, T_SEMI);
			return (res);
		}
	}
}

// Parse one top-level function
// Sets the func_name param.
static struct Afunction* function(struct Pcontext *ctx) {
	struct Afunction *res = afunc_make();

	match(ctx, T_INT);
	expect(ctx, T_ID);
	res->name = current(ctx)->val_s;	// transfer ownership of the identifier string to caller
	current(ctx)->val_s = NULL;	// prevent it from being freed in token_free() called by next(ctx).
	next(ctx);

	match(ctx, T_LP);
	if (current(ctx)->type == T_VOID) {
		next(ctx);
		goto END_PARAM_LIST;
	}
	// TODO: parameter list

END_PARAM_LIST:
	match(ctx, T_RP);
	res->rt = block(ctx);
	return (res);
}

// Frees a Pcontext and all its components.
static void Pcontext_free(struct Pcontext *ctx) {
	struct llist_node *p = ctx->tokens.head, *nxt;
	while (p) {
		nxt = p->nxt;
		token_free((void*)p);
		p = nxt;
	}
}

// Parse source into AST.
struct Afunction* Afunction_from_source(const char *filename) {
	struct Pcontext ctx = {
		.tokens = scan_tokens(filename),
	};
	ctx.cur = ctx.tokens.head;

	struct Afunction* res = function(&ctx);
	Pcontext_free(&ctx);
	return (res);
}
