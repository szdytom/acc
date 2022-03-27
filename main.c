#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include "scan.h"

// List of printable tokens
char *token_str[] = {  "+", "-", "*", "/", "intlit" };

// Loop scanning in all the tokens in the input file.
// Print out details of each token found.
static void scanfile(void) {
	struct token T;
	while (scan(&T)) {
		printf("Token %s", token_str[T.token]);
		if (T.token == T_INTLIT) {
			printf(", value %d", T.intval);
		}
		printf("\n");
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
	scanfile();
	return (0);
}
