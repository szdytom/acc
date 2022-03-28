#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include "defs.h"

int Preview;
int Line = 1;
FILE *Infile;
char *Text;

const char *token_typename[] = { "EOF", ";", "=", "+", "-", "*", "/", "print", "int", "integer literal", "indentifier" };

// open input file
void open_inputfile(char *filename) {
	Infile = fopen(filename, "r");
	if (Infile == NULL) {
		fprintf(stderr, "Unable to open %s: %s\n", filename, strerror(errno));
		exit(1);
	}
}

// close Infile and free vars
void scan_unload(void) {
	if (Infile) {
		fclose(Infile);
	}
	if (Text) {
		free(Text);
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
// i.e. whitespace, newlines.
static void skip_whitespaces(void) {
	int c;

	c = preview();
	while (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f') {
		next();
		c = preview();
	}
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

// Scan an identifier from the input file and
// Return the identifier (char*)
static char* scan_indentifier(int *n) {
	int sz = 128, len = 0;

	char *res = malloc(sz * sizeof(char));
	memset(res, 0, sz * sizeof(char));

	int c = preview();
	while (isdigit(c) || isalpha(c) || c == '_') {
		if (len >= sz - 1) {
			sz *= 2;
			char *old = res;
			res = malloc(sz * sizeof(char));
			memcpy(res, old, len * sizeof(char));
			memset(res + len * sizeof(char), 0, (sz - len) * sizeof(char));
			free(old);
		}
		res[len++] = c;
		next();
		c = preview();
	}

	if (n) {
		*n = len;
	}
	return (res);
}

// Given a word from the input, scan if it is a keyword
static int scan_keyword(struct token *t, char *s) {
	if (s[0] == 'p') {
		if (!strcmp(s, "print")) { //equal
			t->type = T_PRINT;
			return (1);
		}
	} else if (s[0] == 'i') {
		if (!strcmp(s, "int")) {
			t->type = T_INT;
			return (1);
		}
	}
	return (0);
}

// Scan and return the next token found in the input.
void scan(struct token *t) {
	skip_whitespaces();
	int c = preview();
	if (c == EOF) {
		t->type = T_EOF;
		return;
	}

	if (c == '+') {
		t->type = T_PLUS;
	} else if (c == '-') {
		t->type = T_MINUS;
	} else if (c == '*') {
		t->type = T_STAR;
	} else if (c == '/') {
		t->type = T_SLASH;
	} else if (c == ';') {
		t->type = T_SEMI;
	} else if (c == '=') {
		t->type = T_EQUAL;
	} else {
		// If it's a digit, scan the literal integer value in
		if (isdigit(c)) {
			t->type = T_INTLIT;
			t->intval = scanint();
		} else if (isalpha(c) || c == '_') {
			if (Text) {
				free(Text);
			}

			Text = scan_indentifier(NULL);
			if (!scan_keyword(t, Text)) {
				// not a keyword, so it should be an indentifier.
				t->type = T_INDENT;
			}
		}
		return;
	}

	next();
}

