#ifndef ACC_AST_H
#define ACC_AST_H

#include <stdint.h>
#include "util/linklist.h"

// AST operation types
enum {
	A_ASSIGN,
	A_ADD, A_SUB, A_MUL, A_DIV,
	A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE,
	A_LIT_I32, A_LIT_I64,
	A_VAR,
	A_BLOCK,
	A_PRINT, A_IF, A_WHILE,
	A_RETURN,
	A_SOUL // what?
};

extern const char *ast_opname[31];

// AST structure field shared by all types 
// llist_node *n	: for linklist
// int op		: node operation
#define ASTnode_SHARED_FIELDS \
	struct llist_node n; \
	int op;

// AST structure (common)
struct ASTnode {
	ASTnode_SHARED_FIELDS 
};

// AST binary operation node
struct ASTbinnode {
	ASTnode_SHARED_FIELDS 
	struct ASTnode *left;
	struct ASTnode *right;
};

// AST if statement node
struct ASTifnode {
	ASTnode_SHARED_FIELDS 
	struct ASTnode *left;	// condition true branch
	struct ASTnode *right;	// condition false branch
	struct ASTnode *cond;
};

// AST unary operation node
struct ASTunnode {
	ASTnode_SHARED_FIELDS 
	struct ASTnode *left;
};

// AST block node
struct ASTblocknode {
	ASTnode_SHARED_FIELDS 
	struct linklist st; // statements linklist
};

// AST integer literal (32bit) node
struct ASTi32node {
	ASTnode_SHARED_FIELDS 
	int32_t val;
};

// AST integer literal (64bit) node
struct ASTi64node {
	ASTnode_SHARED_FIELDS 
	int64_t val;
};

// AST assign literal node
struct ASTassignnode {
	ASTnode_SHARED_FIELDS 
	struct ASTnode* left;
	struct ASTnode* right;
};

// AST variable value node
struct ASTvarnode {
	ASTnode_SHARED_FIELDS 
	int id;
};

struct ASTnode* ast_make_binary(int op, struct ASTnode *left, struct ASTnode *right);
struct ASTnode* ast_make_lit_i32(int32_t x);
struct ASTnode* ast_make_lit_i64(int64_t x);
struct ASTnode* ast_make_unary(int op, struct ASTnode *c);
struct ASTnode* ast_make_block();
struct ASTnode* ast_make_var(int id);
struct ASTnode* ast_make_assign(int op, struct ASTnode *left, struct ASTnode *right);
struct ASTnode* ast_make_if(struct ASTnode *left, struct ASTnode *right, struct ASTnode *cond);
void ast_free(struct ASTnode *x);

#endif

