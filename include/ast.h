#ifndef ACC_AST_H
#define ACC_AST_H

#include "util/linklist.h"

// AST operation types
enum {
	A_ASSIGN,
	A_ADD, A_SUB, A_MUL, A_DIV,
	A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE,
	A_INTLIT, A_VAR,
	A_BLOCK,
	A_PRINT, A_IF, A_WHILE,
	A_SOUL // what?
};

// AST nodde types
enum {
	N_BIN, N_UN, N_MULTI, N_LEAF, N_ASSIGN
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

// AST if statement node
struct ASTifnode {
	int op;
	struct ASTnode *left;	// condition true branch
	struct ASTnode *right;	// condition false branch
	struct ASTnode *cond;
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

// AST assign literal node
struct ASTassignnode {
	int op;
	int left;
	struct ASTnode* right;
};

// AST variable value node
struct ASTvarnode {
	int op;
	int id;
};

struct ASTnode* ast_make_binary(int op, struct ASTnode *left, struct ASTnode *right);
struct ASTnode* ast_make_intlit(int val);
struct ASTnode* ast_make_unary(int op, struct ASTnode *c);
struct ASTnode* ast_make_block();
struct ASTnode* ast_make_var(int id);
struct ASTnode* ast_make_assign(int op, int left, struct ASTnode *right);
struct ASTnode* ast_make_if(struct ASTnode *left, struct ASTnode *right, struct ASTnode *cond);
int ast_type(int t);
void ast_free(struct ASTnode *x);

#endif

