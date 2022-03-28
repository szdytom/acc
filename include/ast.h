#ifndef ACC_AST_H
#define ACC_AST_H

#include "defs.h"

struct ASTnode* make_astnode(int op, struct ASTnode *left, struct ASTnode *right, int intval);
struct ASTnode* make_leaf(int op, int intval);
struct ASTnode* make_unary(int op, struct ASTnode *c, int intval);
void free_ast(struct ASTnode *rt);

#endif
