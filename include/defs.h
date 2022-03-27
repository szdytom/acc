#ifndef ACC_DEFS_H
#define ACC_DEFS_H

// Token structure
struct token {
	int token;	// token type
	int intval;	// hold the value of the integer that we scanned in
};

// Tokens
enum {
	T_PLUS, T_MINUS, T_STAR, T_SLASH,
	T_INTLIT
};

#endif
