#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scan.h"
#include "parse.h"
#include "ast.h"
#include "target.h"
#include "quad.h"

// Print out a usage if started incorrectly
static void usage(char *prog) {
	fprintf(stderr, "ACC the C compiler. built on: %s.\n", __DATE__);
	fprintf(stderr, "Usage: %s target infile (outfile)\n", prog);
	exit(1);
}

static FILE *Outfile;

// Do clean up job
void unload(void) {
	if (Outfile && Outfile != stdout) {
		fclose(Outfile);
	}
}

int main(int argc, char *argv[]) {
	atexit(unload);
	if (argc < 3) {
		usage(argv[0]);
	}

	if (argc >= 4) {
		Outfile = fopen(argv[3], "w");
	} else {
		Outfile = stdout;
	}

	int target = target_parse(argv[1]);
	struct Afunction *afunc = parse_source(argv[2]);
	if (target == TARGET_AST) {
		afunc_debug_print(Outfile, afunc);
	} else if (target == TARGET_QUAD) {
		struct Qfunction *qfunc = qfunc_cgenerate(afunc);
		qfunc_debug_print(qfunc, Outfile);
		qfunc_free(qfunc);
	}
	afunc_free(afunc);
	return (0);
}
