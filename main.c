#include <stdio.h>
#include <stdlib.h>
#include "scan.h"
#include "defs.h"
#include "parse.h"
#include "cg.h"
#include "ast.h"
#include "symbol.h"

// Print out a usage if started incorrectly
static void usage(char *prog) {
	fprintf(stderr, "Usage %s infile outfile\n", prog);
	exit(1);
}

void unload(void) {
	scan_unload();
	cg_unload();
	symbol_unload();
}

// Main program: check arguments and print a usage
// if we don't have an argument. Open up the input
// file and call scanfile() to scan the tokens in it.
int main(int argc, char *argv[]) {
	atexit(unload);
	if (argc == 1) {
		usage(argv[0]);
	}
	open_inputfile(argv[1]);
	if (argc == 2) {
		open_outputfile("out.s");
	} else {
		open_outputfile(argv[2]);
	}

	symbol_init();
	struct ASTnode *rt = parse();
	cg_main(rt);
	free_ast(rt);
	return (0);
}
