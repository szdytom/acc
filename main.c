#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scan.h"
#include "parse.h"
#include "ast.h"
#include "target.h"
#include "print_ast.h"

// Print out a usage if started incorrectly
static void usage(char *prog) {
	fprintf(stderr, "ACC the C compiler. built on: %s.\n", __DATE__);
	fprintf(stderr, "Usage: %s target infile (outfile)\n", prog);
	exit(1);
}

// Do clean up job
void unload(void) {
}

int main(int argc, char *argv[]) {
	atexit(unload);
	if (argc < 3) {
		usage(argv[0]);
	}

	FILE *Outfile;
	if (argc >= 4) {
		Outfile = fopen(argv[3], "w");
	} else {
		Outfile = fopen("out.txt", "w");
	}

	int target = target_parse(argv[1]);
	struct ASTnode *rt = parse(argv[2]);
	if (target == TARGET_AST) {
		debug_ast_print(Outfile, rt);
	}
	ast_free(rt);
	fclose(Outfile);
	return (0);
}
