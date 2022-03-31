#ifndef ACC_TOKEN_H
#define ACC_TOKEN_H

#include "util/linklist.h"

// Token structure
struct token {
	int type;	// token type
	int intval;	// hold the value of the integer that we scanned in
};

// Tokens
enum {
	T_EOF,
	T_SEMI,
	T_LB, T_RB, T_LP, T_RP,
	T_ASSIGN,
	T_PLUS, T_MINUS, T_STAR, T_SLASH,
	T_EQ, T_NE, T_LT, T_GT, T_LE, T_GE,
	T_PRINT, T_INT, T_IF, T_ELSE, T_WHILE, T_FOR,
	T_INTLIT, T_INDENT,
};
extern const char *token_typename[25];

#endif
