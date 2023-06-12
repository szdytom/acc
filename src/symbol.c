/*
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "util/misc.h"
#include "util/array.h"

#define INDENT_CHARS 63 // 10 digits + 26 * 2(cap) alpha + 1 _

static int indent_char_id(char c) {
	if ('0' <= c && c <= '9') {
		return (c - '0');
	}
	if ('a' <= c && c <= 'z') {
		return (c - 'a' + 10);
	}
	if ('A' <= c && c <= 'Z') {
		return (c - 'A' + 36);
	}
	if (c == '_') {
		return (62);
	}
	fprintf(stderr, "%c is not a acceptable char in indentifier.\n", c);
	exit(1);
}

struct trie_node {
	struct trie_node *c[INDENT_CHARS];	// childs
	int sz;		// size
	int val;	// endpoint value, -1 for not ended
};

static struct trie_node *root;
struct array Gsym;	// symbol id to name map

static void trie_free(struct trie_node *p) {
	for (int i = 0; i < INDENT_CHARS; ++i) {
		if (p->c[i]) {
			trie_free(p->c[i]);
		}
	}
	free(p);
}

static struct trie_node* trie_createnode(void) {
	struct trie_node *x = malloc(sizeof(struct trie_node));
	if (x == NULL) {
		fprintf(stderr, "%s: failed to malloc trie node in symbol table.\n", __FUNCTION__);
		exit(1);
	}

	memset(x, 0, sizeof(struct trie_node));
	x->val = -1;
	return (x);
}

static void trie_set(char *str, int val) {
	struct trie_node *p = root;

	int n = strlen(str);
	for (int i = 0; i < n; ++i) {
		int x = indent_char_id(str[i]);
		if (!p->c[x]) {
			p->c[x] = trie_createnode();
		}
		p->sz += 1;
		p = p->c[x];
	}
	p->val = val;
}

static int trie_get(char *str) {
	struct trie_node *p = root;

	int n = strlen(str);
	for (int i = 0; i < n; ++i) {
		int x = indent_char_id(str[i]);
		if (!p->c[x]) { // not found
			return (-1);
		}
		p = p->c[x];
	}
	return (p->val);
}

static int IsSymbolListLoaded = 0;

// init global symbol table
void symbol_init(void) {
	IsSymbolListLoaded = 1;
	array_init(&Gsym);
	root = trie_createnode();
}

// unload global symbol table
void symbol_unload(void) {
	if (!IsSymbolListLoaded) {
		return;
	}

	for (int i = 0; i < Gsym.length; ++i) {
		free(array_get(&Gsym, i));
	}
	array_free(&Gsym);
	trie_free(root);
}

// Determine if the symbol s is in the global symbol table.
// Return its slot position or -1 if not found.
int findglob(char *s) {
	return (trie_get(s));
}

// Add a global symbol to the symbol table.
// Return the slot number in the symbol table.
int addglob(char *s) {
	char *ss = strclone(s);
	array_pushback(&Gsym, ss);
	int res = Gsym.length - 1;
	trie_set(ss, res);
	return (res);
}
*/
