#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scan.h"
#include "defs.h"
#include "parse.h"
#include "cg.h"
#include "ast.h"
#include "symbol.h"

// Print out a usage if started incorrectly
static void usage(char *prog) {
	fprintf(stderr, "Usage %s target infile (outfile)\n", prog);
	exit(1);
}

//Do cClean up job
void unload(void) {
	scan_unload();
	cg_unload();
	symbol_unload();
}

int main(int argc, char *argv[]) {
	atexit(unload);
	if (argc < 3) {
		usage(argv[0]);
	}

	int outfile_opened = 0;
	if (argc >= 4) {
		open_outputfile(argv[3]);
		outfile_opened = 1;
	}

	int target;
	if (!strcmp(argv[1], "x86_64")) {
		target = CG_X64;
		if (!outfile_opened) {
			open_outputfile("out.s");
		}
	} else if (!strcmp(argv[1], "llvm")) {
		target = CG_LLVM;
		if (!outfile_opened) {
			open_outputfile("out.ll");
		}
	} else {
		fprintf(stderr, "Unknow target %s.\n", argv[1]);
	}

	open_inputfile(argv[2]);

	symbol_init();
	struct ASTnode *rt = parse();
	cg_main(target, rt);
	free_ast(rt);
	return (0);
}
