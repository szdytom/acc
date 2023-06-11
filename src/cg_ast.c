#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "cg.h"
#include "symbol.h"
#include "fatals.h"
#include "util/array.h"

static const char *ast_opname[] = {
	"=",
	"+", "-", "*", "/",
	"==", "!=", "<", ">", "<=", ">=",
	"int", "var",
	"block",
	"print", "if", "while"
};

static int tabs;

static void cgprint_tabs() {
	for (int i = 0; i < tabs; ++i) {
		fprintf(Outfile, "\t");
	}
}

static void cgenerate_dfs(struct ASTnode *x) {
	if (x == NULL) {
		cgprint_tabs();
		fprintf(Outfile, "--->NULL.\n");
		return;
	}

	int nt = ast_type(x->op);
	if (nt == N_LEAF) {
		if (x->op == A_LIT) {
			struct ASTlitnode *t = (struct ASTlitnode*)x;
			cgprint_tabs();
			switch (t->type.vt) {
			case V_I32:
				fprintf(Outfile, "--->INT %d.\n", *(int32_t*)t->val);
				break;
			case V_I64:
				fprintf(Outfile, "--->INT64 %lld.\n", *(int64_t*)t->val);
				break;
			default:
				fprintf(stderr, "%s: Unknow literal type %d.\n", __FUNCTION__, t->type.vt);
				exit(1);
			}
		} else if (x->op == A_VAR) {
			struct ASTvarnode *t = (struct ASTvarnode*)x;
			cgprint_tabs();
			fprintf(Outfile, "--->VAR @%s.\n", (char*)array_get(&Gsym, t->id));
		} else {
			fail_ast_op(x->op, __FUNCTION__);
		}
	} else if (nt == N_ASSIGN) {
		struct ASTassignnode *t = (struct ASTassignnode*)x;
		cgprint_tabs();
		fprintf(Outfile, "--->ASSIGN(%s) to @%s (right)\n", ast_opname[t->op], (char*)array_get(&Gsym, t->left));
		tabs += 1;
		cgenerate_dfs(t->right);
		tabs -= 1;
	} else if (nt == N_BIN) {
		if (x->op == A_IF) {
			struct ASTifnode *t = (struct ASTifnode*)x;
			cgprint_tabs();
			fprintf(Outfile, "--->IF (cond left right)\n");
			tabs += 1;
			cgenerate_dfs(t->cond);
			cgenerate_dfs(t->left);
			cgenerate_dfs(t->right);
			tabs -= 1;
		} else if (x->op == A_WHILE) {
			struct ASTbinnode *t = (struct ASTbinnode*)x;
			cgprint_tabs();
			fprintf(Outfile, "--->WHILE(%s) (cond body)\n", ast_opname[t->op]);
			tabs += 1;
			cgenerate_dfs(t->left);
			cgenerate_dfs(t->right);
			tabs -= 1;
		} else {
			struct ASTbinnode *t = (struct ASTbinnode*)x;
			cgprint_tabs();
			fprintf(Outfile, "--->BINOP(%s) (left right)\n", ast_opname[t->op]);
			tabs += 1;
			cgenerate_dfs(t->left);
			cgenerate_dfs(t->right);
			tabs -= 1;
		}
	} else if (nt == N_UN) {
		if (x->op == A_PRINT) {
			struct ASTunnode *t = (struct ASTunnode*)x;
			cgprint_tabs();
			fprintf(Outfile, "--->PRINT (value)\n");
			tabs += 1;
			cgenerate_dfs(t->c);
			tabs -= 1;
		} else {
			fail_ast_op(x->op, __FUNCTION__);
		}
	} else if (nt == N_MULTI) {
		struct ASTblocknode *t = (struct ASTblocknode*)x;
		cgprint_tabs();
		fprintf(Outfile, "--->BLOCK (%d childs)\n", t->st.length);
		tabs += 1;
		struct llist_node *p = t->st.head;
		while (p) {
			cgenerate_dfs(p->val);
			p = p->nxt;
		}
		tabs -= 1;
	}
}

void cgast_generate(struct ASTnode *rt) {
	tabs = 0;
	cgenerate_dfs(rt);
}

