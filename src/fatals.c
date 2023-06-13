#include <stdio.h>
#include <stdlib.h>
#include "scan.h"
#include "ast.h"
#include "quad.h"

void fail_target(const char *target_name) {
	fprintf(stderr, "unknown target: %s.\n", target_name);
	exit(1);
}

void fail_malloc(const char *func_name) {
	fprintf(stderr, "%s: unable to malloc.\n", func_name);
	exit(1);
}

void* malloc_or_fail(size_t s, const char *func_name) {
	void *res = malloc(s);
	if (res == NULL) {
		fail_malloc(func_name);
	}
	return (res);
}

void fail_quad_op(int op, const char *func_name) {
	if (op < Q_NULL) {
		fprintf(stderr, "%s: unsupported Quad operator %s.\n", func_name, ast_opname[op]);
		exit(1);
	} else {
		fprintf(stderr, "%s: unknown Quad operator %d.\n", func_name, op);
		exit(1);
	}
}

void fail_ast_op(int op, const char *func_name) {
	if (op < A_SOUL) {
		fprintf(stderr, "%s: unsupported AST operator %s.\n", func_name, ast_opname[op]);
		exit(1);
	} else {
		fprintf(stderr, "%s: unknown AST operator %d.\n", func_name, op);
		exit(1);
	}
}

void fail_ce_expect(int line, const char *expected, const char *got) {
	fprintf(stderr, "syntax error on line %d: expected %s, got %s.\n", line, expected, got);
	exit(1);
}

void fail_ce(int line, const char *reason) {
	fprintf(stderr, "syntax error on line %d: %s.\n", line, reason);
	exit(1);
}

void fail_char(int line, int c) {
	fprintf(stderr, "Unrecognised character %c on line %d.\n", c, line);
	exit(1);
}

