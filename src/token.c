#include <stdlib.h>
#include <stdio.h>
#include "token.h"

const char *token_typename[63] = {
	"EOF",
	";",
	"{", "}", "(", ")",
	"=",
	"+", "-", "*", "/",
	"==", "!=", "<", ">", "<=", ">=",
	"int", "void", "char", "long",
	"print", "if", "else", "while", "for",
	"a signed integer literal (size 32)", "a signed integer literal (size 64)", "an indentifier"
};

void token_free(struct token *t) {
	if (t->val) {
		free(t->val);
	}
	free(t);
}

struct token token_make_eof(void) {
	struct token res;
	res.type = T_EOF;
	res.val = NULL;
	return (res);
}