#include <stdlib.h>
#include <stdio.h>
#include "token.h"

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