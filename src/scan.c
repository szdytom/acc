#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include "token.h"
#include "fatals.h"
#include "util/misc.h"

int Line = 1;
static int Preview;
static FILE *Infile;

const char *token_typename[] = {
	"EOF",
	";",
	"{", "}", "(", ")",
	"=",
	"+", "-", "*", "/",
	"==", "!=", "<", ">", "<=", ">=",
	"int", "void", "char", "long",
	"print", "if", "else", "while", "for",
	"an integer literal (type int)", "an integer literal (type long)", "an indentifier"
};

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
static void scanint(struct token *t) {
	long long res = 0;
	int c = preview();
	while ('0' <= c && c <= '9') {
		res = res * 10 + (c - '0');
		next();
		c = preview();
	}

	if (INT_MIN <= res && res <= INT_MAX) {
		t->type = T_INTLIT;
		t->val = malloc(sizeof(int));
		if (t->val == NULL) {
			fail_malloc(__FUNCTION__);
		}
		*((int *)t->val) = (int)res;
	} else {
		t->type = T_LONGLIT;
		t->val = malloc(sizeof(long long));
		if (t->val == NULL) {
			fail_malloc(__FUNCTION__);
		}
		*((long long *)t->val) = res;
	}
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
	static const char *map_s[] = {
		"print",
		"int",
		"void",
		"long",
		"if",
		"else",
		"while",
		"for",
		NULL
	};

	static const int map_t[] = {
		T_PRINT,
		T_INT,
		T_VOID,
		T_LONG,
		T_IF,
		T_ELSE,
		T_WHILE,
		T_FOR,
		-1
	};

	for (int i = 0; map_s[i] != NULL; ++i) {
		if (strequal(map_s[i], s)) {
			t->type = map_t[i];
			return (1);
		}
	}
	return (0);
}

// Scan one char token
// Return 1 if found
static int scan_1c(struct token *t) {
	static const int map[][2] = {
		{'+', T_PLUS},
		{'-', T_MINUS},
		{'*', T_STAR},
		{'/', T_SLASH},
		{'{', T_LB},
		{'}', T_RB},
		{'(', T_LP},
		{')', T_RP},
		{';', T_SEMI},
		{'\0', -1}
	};

	int c = preview();
	for (int i = 0; map[i][0] != '\0'; ++i) {
		if (map[i][0] == c) {
			t->type = map[i][1];
			next();
			return (1);
		}
	}
	return (0);
}

// Scan and return the next token found in the input.
static struct token* scan(void) {
	struct token *t = malloc(sizeof(struct token));
	if (t == NULL) {
		fail_malloc(__FUNCTION__);
	}
	t->val = NULL;

	skip_whitespaces();
	int c = preview();
	if (c == EOF) {
		t->type = T_EOF;
		return (t);
	}

	if (scan_1c(t)) {
		return (t);
	}

	if (c == '=') {
		t->type = T_ASSIGN;
		next();
		c = preview();
		if (c == '=') {
			t->type = T_EQ;
			next();
		}
	} else if (c == '!') {
		next();
		c = preview();
		if (c == '=') {
			t->type = T_NE;
			next();
		} else {
			fprintf(stderr, "Unrecognised character %c on line %d.\n", c, Line);
			exit(1);
		}
	} else if (c == '<') {
		t->type = T_LT;
		next();
		c = preview();
		if (c == '=') {
			t->type = T_LE;
			next();
		}
	} else if (c == '>') {
		t->type = T_GT;
		next();
		c = preview();
		if (c == '=') {
			t->type = T_GE;
			next();
		}
	} else {
		// If it's a digit, scan the integer literal value in
		if (isdigit(c)) {
			scanint(t);
		} else if (isalpha(c) || c == '_') {
			t->val = scan_indentifier(NULL);
			if (scan_keyword(t, t->val)) {
				// got a keyword
				free(t->val);
				t->val = NULL;
			} else {
				// not a keyword, so it should be an indentifier.
				t->type = T_INDENT;
			}
		} else {
			fail_char(c);
		}
	}
	return (t);
}

struct linklist scan_tokens(const char *name) {
	Infile = fopen(name, "r");
	if (Infile == NULL) {
		fprintf(stderr, "Cannot open file %s.\n", name);
		exit(1);
	}

	struct linklist res;
	llist_init(&res);
	while (1) {
		struct token *t = scan();
		llist_pushback(&res, t);
		if (t->type == T_EOF) {
			break;
		}
	}

	fclose(Infile);
	return (res);
}