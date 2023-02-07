#include <stdio.h>
#include <stdlib.h>
#include "scan.h"

void fail_malloc(const char *func_name) {
	fprintf(stderr, "%s: unable to malloc.\n", func_name);
	exit(1);
}

void fail_ast_op(int op, const char *func_name) {
	fprintf(stderr, "%s: unknown ast operator %d.\n", func_name, op);
	exit(1);
}

void fail_ce_expect(const char *expected, const char *got) {
	fprintf(stderr, "syntax error on line %d: expected %s, got %s.\n", Line, expected, got);
	exit(1);
}

void fail_ce(const char *reason) {
	fprintf(stderr, "syntax error on line %d: %s.\n", Line, reason);
	exit(1);
}

void fail_char(int c) {
	fprintf(stderr, "Unrecognised character %c on line %d.\n", c, Line);
	exit(1);
}

