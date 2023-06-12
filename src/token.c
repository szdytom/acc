#include <stdlib.h>
#include "token.h"

const char *token_typename[63] = {
	"EOF",
	";",
	"{", "}", "(", ")",
	"=",
	"+", "-", "*", "/",
	"==", "!=", "<", ">", "<=", ">=",
	"int", "void", "char", "long",
	"short",
	"print", "if", "else",
	"while", "for",
	"return",
	"a integer literal (16bit)", "a integer literal (32bit)", "a integer literal (64bit)",
	"an identifier",
	NULL
};

void token_free(struct token *t) {
	if (t->type == T_ID && t->val_s) {
		free(t->val_s);
	}
	free(t);
}
