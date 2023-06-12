#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "print_ast.h"
#include "symbol.h"
#include "fatals.h"
#include "util/array.h"

static int tabs;

static void print_tabs(FILE* Outfile) {
	for (int i = 0; i < tabs; ++i) {
		fprintf(Outfile, "\t");
	}
}

static void ast_dfs(FILE* Outfile, struct ASTnode *x) {
	print_tabs(Outfile);
	if (x == NULL) {
		fprintf(Outfile, "--->NULL.\n");
		return;
	}

	switch(x->op) {
		case A_RETURN: case A_PRINT: {
			struct ASTunnode *t = (struct ASTunnode*)x;
			fprintf(Outfile, "--->UNOP(%s)\n", ast_opname[x->op]);
			tabs += 1;
			ast_dfs(Outfile, t->left);
			tabs -= 1;
		}	break;

		case A_LIT_I32: {
			struct ASTi32node *t = (struct ASTi32node*)x;
			fprintf(Outfile, "--->INT32(%d)\n", t->val);
		}	break;

		case A_LIT_I64: {
			struct ASTi64node *t = (struct ASTi64node*)x;
			fprintf(Outfile, "--->INT64(%lld)\n", t->val);
		}	break;

		case A_BLOCK: {
			struct ASTblocknode *t = (struct ASTblocknode*)x;
			fprintf(Outfile, "--->BLOCK(%d statements)\n", t->st.length);
			tabs += 1;
			struct llist_node *p = t->st.head;
			while (p) {
				ast_dfs(Outfile, (struct ASTnode*)p);
				p = p->nxt;
			}
			tabs -= 1;
		}	break;

		default: {
			fprintf(Outfile, "--->%s...\n", ast_opname[x->op]);
		}	break;
	}
}

void debug_ast_print(FILE *Outfile, struct ASTnode *rt) {
	tabs = 0;
	ast_dfs(Outfile, rt);
}

