#ifndef ACC_AST_H
#define ACC_AST_H

#include "defs.h"

struct ASTnode* ast_make_binary(int op, struct ASTnode *left, struct ASTnode *right);
struct ASTnode* ast_make_intlit(int val);
struct ASTnode* ast_make_unary(int op, struct ASTnode *c);
struct ASTnode* ast_make_block();
int ast_type(int t);
void free_ast(struct ASTnode *x);

#endif
