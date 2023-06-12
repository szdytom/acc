#ifndef ACC_TOKEN_H
#define ACC_TOKEN_H

#include <stdint.h>
#include "util/linklist.h"

// Token structure
struct token {
	struct llist_node n;
	int type;	// token type
	union {		// hold the value of the literal that we scanned in
		int16_t val_i16;
		int32_t val_i32;
		int64_t val_i64;
		char *val_s;
	};
};

// Tokens
enum {
	T_EOF,
	T_SEMI,					// ;
	T_LB, T_RB, T_LP, T_RP,			// { } ( )
	T_ASSIGN,				// =
	T_PLUS, T_MINUS, T_STAR, T_SLASH,	// + - * /
	T_EQ, T_NE, T_LT, T_GT, T_LE, T_GE,	// == != < > <= >=
	T_INT, T_VOID, T_CHAR, T_LONG,		// int void char long
	T_SHORT,				// short
	T_PRINT, T_IF, T_ELSE,			// print if else
	T_WHILE, T_FOR,				// while for
	T_RETURN,				// return
	T_I16_LIT, T_I32_LIT, T_I64_LIT,
	T_ID,
	T_EXCEED,
};
extern const char *token_typename[63];

void token_free(struct token *t);

#endif

