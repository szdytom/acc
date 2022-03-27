#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include "defs.h"

int Preview;
int Line = 1;
FILE *Infile;

// open input file
void openfile(char *filename) {
	Infile = fopen(filename, "r");
	if (Infile == NULL) {
		fprintf(stderr, "Unable to open %s: %s\n", filename, strerror(errno));
		exit(1);
	}
}

// preview one char, not getting it out from the stream
static int preview(void) {
	if (!Preview) {
		Preview = fgetc(Infile);
	}
	return (Preview);
}

// Get the next char from the input file
static void next(void) {
	int c = preview();
	Preview = 0;
	if (c == '\n') {
		Line += 1;
	}
}

// Skip past input that we don't need to deal with, 
// i.e. whitespace, newlines. Return the first
// character we do need to deal with.
static int skip_whitespaces(void) {
	int c;

	c = preview();
	while (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f') {
		next();
		c = preview();
	}
	return (c);
}

// Scan and return an integer literal value from the input file. 
static int scanint() {
	int res = 0;

	int c = preview();
	while ('0' <= c && c <= '9') {
		res = res * 10 + (c - '0');
		next();
		c = preview();
	}
	return (res);
}

// Scan and return the next token found in the input.
// Return 1 if token valid, 0 if no tokens left.
int scan(struct token *t) {
	int c = skip_whitespaces();
	if (c == EOF) {
		t->token = T_EOF;
		return (0);
	} else if (c == '+') {
		t->token = T_PLUS;
		next();
	} else if (c == '-') {
		t->token = T_MINUS;
		next();
	} else if (c == '*') {
		t->token = T_STAR;
		next();
	} else if (c == '/') {
		t->token = T_SLASH;
		next();
	} else {
		// If it's a digit, scan the
		// literal integer value in
		if (isdigit(c)) {
			t->token = T_INTLIT;
			t->intval = scanint();
		} else {
			fprintf(stderr, "Unrecognised character %c on line %d\n", c, Line);
			exit(1);
		}
	}

	return (1);
}

