#ifndef ACC_TOKEN_H
#define ACC_TOKEN_H

#include "util/linklist.h"

// Token structure
struct token {
	int type;	// token type
	void* val;	// hold the value of the literal that we scanned in
};

// Tokens
enum {
	T_EOF,
	T_SEMI,
	T_LB, T_RB, T_LP, T_RP,
	T_ASSIGN,
	T_PLUS, T_MINUS, T_STAR, T_SLASH,
	T_EQ, T_NE, T_LT, T_GT, T_LE, T_GE,
	T_INT, T_VOID, T_CHAR, T_LONG,
	T_PRINT, T_IF, T_ELSE, T_WHILE, T_FOR,
	T_I32_LIT, T_I64_LIT, T_INDENT,
};
extern const char *token_typename[63];

void token_free(struct token *t);
struct token token_make_eof(void);

#endif
