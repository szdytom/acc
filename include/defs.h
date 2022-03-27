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
	T_INTLIT,
	T_EOF
};

// AST nodes
enum {
	A_ADD, A_SUB, A_MUL, A_DIV, A_INTLIT
};

// AST structure
struct ASTnode {
	int op;	//operator
	struct ASTnode *left;	// left child
	struct ASTnode *right;	// right child
	int intval;	// for A_INTLIT, an integer value
};

#endif
