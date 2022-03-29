#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cg.h"
#include "defs.h"
#include "ast.h"
#include "symbol.h"
#include "util/array.h"
#include "util/linklist.h"

// List of available registers
// and their names
static const int reg_count = 4;
static char *reglist[4] = { "%r8", "%r9", "%r10", "%r11" };
static int usedreg[4];

// Set all registers free
static void free_all_reg(void) {
	for (int i = 0; i < reg_count; ++i) {
		usedreg[i] = 0;
	}
}

static int alloc_reg(void) {
	for (int i = 0; i < reg_count; ++i) {
		if (!usedreg[i]) {
			usedreg[i] = 1;
			return (i);
		}
	}
	fprintf(stderr, "Out of registers.\n");
	exit(0);
}

// Return a register to the list of available registers.
// Check to see if it's not already there.
static void free_reg(int r) {
	if (r == -1) {
		return;
	}

	if (!usedreg[r]) {
		fprintf(stderr, "Error trying to free register %d: not allocated.\n", r);
		exit(1);
	}
	usedreg[r] = 0;
}

// Print out the assembly preamble
static void cgpreamble(void) {
	free_all_reg();
	fputs(	"\t.text\n"
		".LC0:\n"
		"\t.string\t\"%d\\n\"\n"
		"printint:\n"
		"\tpushq\t%rbp\n"
		"\tmovq\t%rsp, %rbp\n"
		"\tsubq\t$16, %rsp\n"
		"\tmovl\t%edi, -4(%rbp)\n"
		"\tmovl\t-4(%rbp), %eax\n"
		"\tmovl\t%eax, %esi\n"
		"\tleaq	.LC0(%rip), %rdi\n"
		"\tmovl	$0, %eax\n"
		"\tcall	printf@PLT\n"
		"\tnop\n"
		"\tleave\n"
		"\tret\n"
		"\n"
		"\t.globl\tmain\n"
		"\t.type\tmain, @function\n"
		"main:\n"
		"\tpushq\t%rbp\n"
		"\tmovq	%rsp, %rbp\n", Outfile);
}

// Print out the assembly postamble
static void cgpostamble(void) {
	fputs(	"\tmovl	$0, %eax\n"
		"\tpopq	%rbp\n"
		"\tret\n", Outfile);
}

// Load an integer literal to a register.
// Return the id of the register
static int cgload_int(int val) {
	int r = alloc_reg();
	fprintf(Outfile, "\tmovq\t$%d, %s\n", val, reglist[r]);
	return (r);
}

// Add two registers together and return
// the number of the register with the result
static int cgadd(int r1, int r2) {
	fprintf(Outfile, "\taddq\t%s, %s\n", reglist[r1], reglist[r2]);
	free_reg(r1);
	return (r2);
}

// Subtract the second register from the first and
// return the number of the register with the result
static int cgsub(int r1, int r2) {
	fprintf(Outfile, "\tsubq\t%s, %s\n", reglist[r2], reglist[r1]);
	free_reg(r2);
	return (r1);
}

// Multiply two registers together and return
// the number of the register with the result
static int cgmul(int r1, int r2) {
	fprintf(Outfile, "\timulq\t%s, %s\n", reglist[r1], reglist[r2]);
	free_reg(r1);
	return (r2);
}

// Divide the first register by the second and
// return the number of the register with the result
static int cgdiv(int r1, int r2) {
	fprintf(Outfile, "\tmovq\t%s,%%rax\n", reglist[r1]);
	fprintf(Outfile, "\tcqo\n");
	fprintf(Outfile, "\tidivq\t%s\n", reglist[r2]);
	fprintf(Outfile, "\tmovq\t%%rax,%s\n", reglist[r1]);
	free_reg(r2);
	return (r1);
}

// Call printint() with the given register
static void cgprint(int r) {
	fprintf(Outfile, "\tmovq\t%s, %%rdi\n", reglist[r]);
	fprintf(Outfile, "\tcall\tprintint\n");
	free_reg(r);
}

// Load a global variable into a register
static int cgload_glob(char *name) {
	int r = alloc_reg();
	fprintf(Outfile, "\tmovq\t%s(%%rip), %s\n", name, reglist[r]);
	return (r);
}

// Store a register's value into a variable
static void cgstore_glob(int r, char *name) {
	fprintf(Outfile, "\tmovq\t%s, %s(%%rip)\n", reglist[r], name);
	free_reg(r);
}

// Compare two registers.
static int cgcompare(int r1, int r2, char *how) {
	fprintf(Outfile, "\tcmpq\t%s, %s\n", reglist[r2], reglist[r1]);
	fprintf(Outfile, "\t%s\t%sb\n", how, reglist[r2]);
	fprintf(Outfile, "\tandq\t$255,%s\n", reglist[r2]);
	free_reg(r1);
	return (r2);
}

// Compare if two registers are equal
static int cgequal(int r1, int r2) {
	return (cgcompare(r1, r2, "sete"));
}

// Compare if two registers are not equal
static int cgnotequal(int r1, int r2) {
	return (cgcompare(r1, r2, "setne"));
}

// Compare if r1 is greater than r2
static int cggreaterthan(int r1, int r2) {
	return (cgcompare(r1, r2, "setg"));
}

// Compare if r1 is less than r2
static int cglessthan(int r1, int r2) {
	return (cgcompare(r1, r2, "setl"));
}

// Compare if r1 is greater or equal than r2
static int cggreaterequal(int r1, int r2) {
	return (cgcompare(r1, r2, "setge"));
}

// Compare if r1 is less or equal than r2
static int cglessequal(int r1, int r2) {
	return (cgcompare(r1, r2, "setle"));
}

// init a global variable
static void cginit_glob(char *name) {
	fprintf(Outfile, "\t.comm\t%s,8,8\n", name);
}

// Given a AST('s root)
// Generate ASM code.
// Return value register id.
static int cgenerate_ast(struct ASTnode *rt) {
	int nt = ast_type(rt->op);

	if (nt == N_LEAF) {
		if (rt->op == A_INTLIT) {
			struct ASTintnode *x = (struct ASTintnode*)rt;
			return (cgload_int(x->val));
		} else if (rt->op == A_VAR) {
			struct ASTvarnode *x = (struct ASTvarnode*)rt;
			return (cgload_glob(array_get(&Gsym, x->id)));
		} else {
			fprintf(stderr, "Unknown AST operator %d.\n", rt->op);
			exit(1);
		}
	} else if (nt == N_BIN) {
		struct ASTbinnode *x = (struct ASTbinnode*)rt;
		int lv = cgenerate_ast(x->left);
		int rv = cgenerate_ast(x->right);

		if (rt->op == A_ADD) {
			return (cgadd(lv, rv));
		} else if (rt->op == A_SUB) {
			return (cgsub(lv, rv));
		} else if (rt->op == A_MUL) {
			return (cgmul(lv, rv));
		} else if (rt->op == A_DIV) {
			return (cgdiv(lv, rv));
		} else if (rt->op == A_EQ) {
			return (cgequal(lv, rv));
		} else if (rt->op == A_NE) {
			return (cgnotequal(lv, rv));
		} else if (rt->op == A_LT) {
			return (cglessthan(lv, rv));
		} else if (rt->op == A_GT) {
			return (cggreaterthan(lv, rv));
		} else if (rt->op == A_LE) {
			return (cglessequal(lv, rv));
		} else if (rt->op == A_GE) {
			return (cggreaterequal(lv, rv));
		} else {
			fprintf(stderr, "Unknown AST operator %d.\n", rt->op);
			exit(1);
		}
	} else if (nt == N_UN) {
		struct ASTunnode *x = (struct ASTunnode*)rt;
		int cv = cgenerate_ast(x->c);

		if (rt->op == A_PRINT) {
			cgprint(cv);
			return (-1);
		} else {
			fprintf(stderr, "Unknown AST operator %d.\n", rt->op);
			exit(1);
		}
	} else if (nt == N_ASSIGN) {
		struct ASTassignnode *x = (struct ASTassignnode*)rt;
		int cv = cgenerate_ast(x->right);

		if (rt->op == A_ASSIGN) {
			cgstore_glob(cv, array_get(&Gsym, x->left));
			return (-1);
		} else {
			fprintf(stderr, "Unknown AST operator %d.\n", rt->op);
			exit(1);
		}
	} else if (nt == N_MULTI) {
		struct ASTblocknode *x = (struct ASTblocknode*)rt;
		int val = -1;
		struct llist_node *p = x->st.head;
		while (p) {
			val = cgenerate_ast(p->val);
			if (p->nxt) {
				free_reg(val);
			}
			p = p->nxt;
		}
		return val;
	} else {
		fprintf(stderr, "Unknown AST operator %d.\n", rt->op);
		exit(1);
	}
}

// generates code
void cgx64_generate(struct ASTnode *rt) {
	for (int i = 0; i < Gsym.length; ++i) {
		cginit_glob(array_get(&Gsym, i));
	}

	cgpreamble();
	free_reg(cgenerate_ast(rt));
	cgpostamble();
}
