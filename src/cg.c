#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "cg.h"

FILE *Outfile;

// open output file of generated code
void open_outputfile(char *filename) {
	Outfile = fopen(filename, "w");
	if (Outfile == NULL) {
		fprintf(stderr, "Unable to open %s: %s\n", filename, strerror(errno));
		exit(1);
	}
}

// close Outfile at exit.
void cg_unload(void) {
	if (Outfile) {
		fclose(Outfile);
	}
}

// generates code
void cg_main(int target, struct ASTnode *rt) {
	if (target == CG_X64) {
		//cgx64_generate(rt);
	} else if (target == CG_LLVM) {
		//cgllvm_generate(rt);
	} else if (target == CG_AST) {
		cgast_generate(rt);
	} else {
		fprintf(stderr, "Unknow target %d.\n", target);
		exit(1);
	}
}

