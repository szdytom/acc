#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "cg.h"
#include "symbol.h"
#include "fatals.h"
#include "util/array.h"
#include "util/linklist.h"

static int alloc_tag(void) {
	static int id = 0;
	return (id++);
}

static int alloc_label(void) {
	static int id = 0;
	return (id++);
}

// Print out the ir preamble
static void cgpreamble(void) {
	fputs(	"@.printint.format = constant [4 x i8] c\"%d\\0A\\00\", align 1\n"
		"declare dso_local i32 @printf(i8* readonly nocapture, ...)\n"
		"define dso_local void @printint(i32 %0) {\n"
		"\t%2 = getelementptr inbounds [4 x i8], [4 x i8]* @.printint.format, i32 0, i32 0\n"
		"\t%3 = call i32 (i8*, ...) @printf(i8* %2, i32 %0)\n"
		"\tret void\n"
		"}\n"
		"\n"
		"define dso_local i32 @main() {\n"
		"entry:\n", Outfile);
}

// Print out the ir postamble
static void cgpostamble(void) {
	fputs("\tret i32 0\n}\n", Outfile);
}

// init as global value
static void cginit_glob(char *name) {
	fprintf(Outfile, "@%s = dso_local global i32 0, align 4\n", name);
}

// Preform arithmetic operation between two i32
static int cgarith_i32(int x, int y, char *op) {
	int r = alloc_tag();
	fprintf(Outfile, "\t%%%d = %s i32 %%%d, %%%d\n", r, op, x, y);
	return (r);
}

// Preform comparision between integers
static int cgcomp_i(int x, int y, char *op, char *ty) {
	int r1 = alloc_tag();
	fprintf(Outfile, "\t%%%d = icmp %s %s %%%d, %%%d\n", r1, op, ty, x, y);
	int r2 = alloc_tag();
	if (strcmp(ty, "i1") != 0) { // special case for bool
		fprintf(Outfile, "\t%%%d = zext i1 %%%d to %s\n", r2, r1, ty);
	}
	return (r2);
}

// Preform comparision between two i32
static int cgcomp_i32(int x, int y, char *op) {
	return (cgcomp_i(x, y, op, "i32"));
}

// Load an int literal
static int cgload_lit_i32(int val) {
	int r = alloc_tag();
	fprintf(Outfile, "\t%%%d = select i1 true, i32 %d, i32 undef\n", r, val);
	return (r);
}

// Load an int from a global variable
static int cgload_glob_i32(char *name) {
	int r = alloc_tag();
	fprintf(Outfile, "\t%%%d = load i32, i32* @%s, align 4\n", r, name);
	return (r);
}

// Store an int into a global variable
static int cgstore_glob_i32(int x, char *name) {
	fprintf(Outfile, "\tstore i32 %%%d, i32* @%s, align 4\n", x, name);
	return (x);
}

// Print a i32
static void cgprint(int x) {
	fprintf(Outfile, "\tcall void (i32) @printint(i32 %%%d)\n", x);
}

// Jump to a label no matter what
static void cgjmp_always(int x) {
	fprintf(Outfile, "\tbr label %%L%d\n", x);
}

// Conditional jump
static void cgjmp_if_i32(int cond, int Lthen, int Lelse) {
	int rc = alloc_tag();
	fprintf(Outfile, "\t%%%d = icmp ne i32 0, %%%d\n", rc, cond);
	fprintf(Outfile, "\tbr i1 %%%d, label %%L%d, label %%L%d\n", rc, Lthen, Lelse);
}

static void cgprint_label(int L) {
	fprintf(Outfile, "L%d:\n", L);
}

// generates llvm ir from ast
static int cgenerate_ast(struct ASTnode *rt) {
	int nt = ast_type(rt->op);
	if (nt == N_LEAF) {
		if (rt->op == A_INTLIT) {
			return (cgload_lit_i32(((struct ASTintnode*)rt)->val));
		} else if (rt->op == A_VAR) {
			return (cgload_glob_i32(array_get(&Gsym, ((struct ASTvarnode*)rt)->id)));
		}
		fail_ast_op(rt->op, __FUNCTION__);
	} else if (nt == N_BIN) {
		if (rt->op == A_IF) {
			struct ASTifnode *x = (struct ASTifnode*)rt;
			int Lthen = alloc_label(), Lelse = alloc_label(), Lend = alloc_label();
			int condv = cgenerate_ast(x->cond);
			cgjmp_if_i32(condv, Lthen, Lelse);

			cgprint_label(Lthen);
			cgenerate_ast(x->left);
			cgjmp_always(Lend);

			cgprint_label(Lelse);
			cgenerate_ast(x->right);
			cgjmp_always(Lend);

			cgprint_label(Lend);
			return (-1);
		} else if (rt->op == A_WHILE) {
			struct ASTbinnode *x = (struct ASTbinnode*)rt;
			int Lstart = alloc_label(), Lbody = alloc_label(), Lend = alloc_label();
			cgjmp_always(Lstart);

			cgprint_label(Lstart);
			int condv = cgenerate_ast(x->left);
			cgjmp_if_i32(condv, Lbody, Lend);

			cgprint_label(Lbody);
			cgenerate_ast(x->right);
			cgjmp_always(Lstart);

			cgprint_label(Lend);
			return (-1);
		}

		struct ASTbinnode *x = (struct ASTbinnode*)rt;
		int lc = cgenerate_ast(x->left);
		int rc = cgenerate_ast(x->right);

		if (rt->op == A_ADD) {
			return (cgarith_i32(lc, rc, "add nsw"));
		} else if (rt->op == A_SUB) {
			return (cgarith_i32(lc, rc, "sub nsw"));
		} else if (rt->op == A_MUL) {
			return (cgarith_i32(lc, rc, "mul nsw"));
		} else if (rt->op == A_DIV) {
			return (cgarith_i32(lc, rc, "sdiv"));
		} else if (rt->op == A_EQ) {
			return (cgcomp_i32(lc, rc, "eq"));
		} else if (rt->op == A_NE) {
			return (cgcomp_i32(lc, rc, "ne"));
		} else if (rt->op == A_GT) {
			return (cgcomp_i32(lc, rc, "sgt"));
		} else if (rt->op == A_GE) {
			return (cgcomp_i32(lc, rc, "sge"));
		} else if (rt->op == A_LT) {
			return (cgcomp_i32(lc, rc, "slt"));
		} else if (rt->op == A_LE) {
			return (cgcomp_i32(lc, rc, "sle"));
		}
		fail_ast_op(rt->op, __FUNCTION__);
	} else if (nt == N_UN) {
		struct ASTunnode *x = (struct ASTunnode*)rt;
		int cv = cgenerate_ast(x->c);

		if (rt->op == A_PRINT) {
			cgprint(cv);
			return (-1);
		}
		fail_ast_op(rt->op, __FUNCTION__);
	} else if (nt == N_ASSIGN) {
		struct ASTassignnode *x = (struct ASTassignnode*)rt;
		int cv = cgenerate_ast(x->right);

		if (rt->op == A_ASSIGN) {
			return (cgstore_glob_i32(cv, array_get(&Gsym, x->left)));
		}
		fail_ast_op(rt->op, __FUNCTION__);
	} else if (nt == N_MULTI) {
		struct ASTblocknode *x = (struct ASTblocknode*)rt;
		int val = -1;
		struct llist_node *p = x->st.head;
		while (p) {
			val = cgenerate_ast(p->val);
			p = p->nxt;
		}
		return val;
	}
	fail_ast_op(rt->op, __FUNCTION__);
}

// generate and write ir to Outfile
void cgllvm_generate(struct ASTnode* rt) {
	for (int i = 0; i < Gsym.length; ++i) {
		cginit_glob(array_get(&Gsym, i));
	}
	cgpreamble();
	cgenerate_ast(rt);
	cgpostamble();
}
