#include <stdio.h>
#include <stdlib.h>
#include "scan.h"
#include "defs.h"
#include "expr.h"

// Given an AST('s root node), 
// interpret the operators in it and return a final value.
static int interpret_ast(struct ASTnode *rt) {
	if (rt->op == A_INTLIT) {
		return rt->intval;
	}

	int lv, rv;
	if (rt->left) {
		lv = interpret_ast(rt->left);
	}

	if (rt->right) {
		rv = interpret_ast(rt->right);
	}

	if (rt->op == A_ADD) {
		return (lv + rv);
	} else if (rt->op == A_SUB) {
		return (lv - rv);
	} else if (rt->op == A_MUL) {
		return (lv * rv);
	} else if (rt->op == A_DIV) {
		return (lv / rv);
	} else {
		fprintf(stderr, "Unknown AST operator %d\n", rt->op);
		exit(1);
	}
}

// Print out a usage if started incorrectly
static void usage(char *prog) {
	fprintf(stderr, "Usage %s infile\n", prog);
	exit(1);
}

// Main program: check arguments and print a usage
// if we don't have an argument. Open up the input
// file and call scanfile() to scan the tokens in it.
int main(int argc, char *argv[]) {
	if (argc == 1) {
		usage(argv[0]);
	}
	openfile(argv[1]);
	printf("%d\n", interpret_ast(parse()));
	return (0);
}
