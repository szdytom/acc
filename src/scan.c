#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "token.h"
#include "fatals.h"
#include "util/misc.h"

int Line = 1;
static int Preview;
static FILE *Infile;

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
	int c = preview();
	while (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f') {
		next();
		c = preview();
	}
}

// Scan and return an integer literal value from the input file.
static void scan_int(struct token *t) {
	int64_t res = 0;
	int c = preview();
	while ('0' <= c && c <= '9') {
		res = res * 10 + (c - '0');
		next();
		c = preview();
	}

	if (INT32_MIN <= res && res <= INT32_MAX) {
		t->type = T_I32_LIT;
		t->val_i32 = (int32_t)res;
	} else {
		t->type = T_I64_LIT;
		t->val_i64 = (int64_t)res;
	}
}

// Scan an identifier from the input file and
// Return the identifier string (char*)
// Writes the length into _n_ or NULL
static char* scan_indentifier(int *n) {
	int sz = 128, len = 0;

	char *res = malloc_or_fail(sz * sizeof(char), __FUNCTION__);
	memset(res, 0, sz * sizeof(char));

	int c = preview();
	while (isdigit(c) || isalpha(c) || c == '_') {
		if (len >= sz - 1) {
			sz *= 2;
			char *res = realloc(res, sz * sizeof(char));
			if (res == NULL) {
				fail_malloc(__FUNCTION__);
			}
			memset(res + len * sizeof(char), 0, (sz - len) * sizeof(char));
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

// Given a word from the input, scan if it is a keyword.
// Returns true if found keyword.
static bool scan_keyword(struct token *t, char *s) {
	static const char *map_s[] = {
		"print",
		"int",
		"void",
		"long",
		"if",
		"else",
		"while",
		"for",
		"return",
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
		T_RETURN,
		T_EXCEED,
	};

	for (int i = 0; map_s[i] != NULL; ++i) {
		if (strequal(map_s[i], s)) {
			t->type = map_t[i];
			return (true);
		}
	}
	return (false);
}

// Scan one char token
// Return 1 if found
static bool scan_1c(struct token *t) {
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
		{'\0', T_EXCEED}
	};

	int c = preview();
	for (int i = 0; map[i][0] != '\0'; ++i) {
		if (map[i][0] == c) {
			t->type = map[i][1];
			next();
			return (true);
		}
	}
	return (false);
}

// Scan and return the next token found in the input.
static struct token* scan(void) {
	struct token *t = malloc_or_fail(sizeof(struct token), __FUNCTION__);

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
			fail_char(c);
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
		if (isdigit(c)) { // If it's a digit, scan the integer literal value in
			scan_int(t);
		} else if (isalpha(c) || c == '_') {
			t->val_s = scan_indentifier(NULL);
			if (scan_keyword(t, t->val_s)) { // got a keyword
				free(t->val_s);
			} else { // not a keyword, so it should be an indentifier.
				t->type = T_ID;
			}
		} else { // cannot match to anything we know, report error.
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

