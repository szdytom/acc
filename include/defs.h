#ifndef ACC_DEFS_H
#define ACC_DEFS_H

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
	T_PLUS, T_MINUS, T_STAR, T_SLASH,
	T_PRINT,
	T_INTLIT,
};
extern const char *token_typename[8];

// AST operation types
enum {
	A_ADD, A_SUB, A_MUL, A_DIV, A_INTLIT, A_BLOCK, A_PRINT
};

// AST nodde types
enum {
	N_BIN, N_UN, N_MULTI, N_LEAF
};

// AST structure (common)
struct ASTnode {
	int op;	//operator
};

// AST binary operation node
struct ASTbinnode {
	int op;
	struct ASTnode *left;
	struct ASTnode *right;
};

// AST unary operation node
struct ASTunnode {
	int op;
	struct ASTnode *c;
};

// AST block node
struct ASTblocknode {
	int op;
	struct linklist st; // statements linklist
};

// AST int literal node
struct ASTintnode {
	int op;
	int val;
};

#endif
