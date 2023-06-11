#ifndef ACC_CG_H
#define ACC_CG_H

#include "ast.h"

extern FILE *Outfile;

// cg.c
void cg_main(int target, struct ASTnode *rt);
void open_outputfile(char *filename);
void cg_unload(void);

// cg_x64.c
//void cgx64_generate(struct ASTnode *rt);

// cg_llvm.c
//void cgllvm_generate(struct ASTnode *rt);

// cg_ast.c
void cgast_generate(struct ASTnode *rt);

// targets
enum {
	CG_X64,		// Intel x86_64
	CG_LLVM,		// LLVM IR
	CG_AST,		// Abstruct Syntax Tree
};

#endif
