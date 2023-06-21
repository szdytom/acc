#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scan.h"
#include "ast.h"
#include "target.h"
#include "acir.h"
#include "util/misc.h"

// Print out a usage if started incorrectly
static void usage(char *prog) {
	fprintf(stderr, "ACC the C compiler. built on: %s.\n", __DATE__);
	fprintf(stderr, "Usage: %s target format infile (outfile)\n", prog);
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
	if (argc < 4) {
		usage(argv[0]);
	}

	if (argc >= 5) {
		Outfile = fopen(argv[4], "w");
	} else {
		Outfile = stdout;
	}

	int target = target_parse(argv[1]);
	Tinfo_load(target);
	struct Afunction *afunc = Afunction_from_source(argv[3]);
	if (strequal(argv[2], "_ast")) {
		Afunction_print(Outfile, afunc);
	} else if (strequal(argv[2], "_ir")) {
		struct IRfunction *ir = IRfunction_from_ast(afunc);
		IRfunction_print(ir, Outfile);
		IRfunction_free(ir);
	}
	Afunction_free(afunc);
	return (0);
}
