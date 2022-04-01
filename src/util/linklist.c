#include <stdlib.h>
#include <stdio.h>
#include "fatals.h"
#include "util/linklist.h"

struct llist_node* llist_createnode(void *val) {
	struct llist_node *res = malloc(sizeof(struct llist_node));
	if (res == NULL) {
		fail_malloc(__FUNCTION__);
	}
	res->nxt = NULL;
	res->val = val;
	return (res);
}

void llist_pushback(struct linklist *l, void *val) {
	l->length += 1;
	if (!l->tail) {
		l->head = l->tail = llist_createnode(val);
		return;
	}
	l->tail->nxt = llist_createnode(val);
	l->tail = l->tail->nxt;
}

void llist_pushback_notnull(struct linklist *l, void *val) {
	if (val) {
		llist_pushback(l, val);
	}
}

void* llist_get(struct linklist *l, int x) {
	if (x >= l->length) {
		fprintf(stderr, "linklist out of range.\n");
		abort();
	}

	struct llist_node *p = l->head;
	for (int i = 0; i < x; ++i) {
		p = p->nxt;
	}
	return (p->val);
}

void llist_set(struct linklist *l, int x, void *val) {
	if (x >= l->length) {
		fprintf(stderr, "linklist out of range.\n");
		abort();
	}

	struct llist_node *p = l->head;
	for (int i = 0; i < x; ++i) {
		p = p->nxt;
	}
	p->val = val;
}

void llist_init(struct linklist *l) {
	l->length = 0;
	l->head = NULL;
	l->tail = NULL;
}

void llist_free(struct linklist *l) {
	struct llist_node *p = l->head;
	struct llist_node *nxt;
	while (p) {
		nxt = p->nxt;
		free(p);
		p = nxt;
	}
	llist_init(l);
}

void llist_insert(struct linklist *l, int index, void *val) {
	if (index >= l->length) {
		llist_pushback(l, val);
		return;
	}

	l->length += 1;
	struct llist_node *x = llist_createnode(val);
	if (index == 0) {
		x->nxt = l->head;
		l->head = x;
		return;
	}

	struct llist_node *p = l->head;
	for (int i = 0; i < index - 1; ++i) {
		p = p->nxt;
	}
	x->nxt = p->nxt;
	p->nxt = x;
}

void llist_popfront(struct linklist *l) {
	if (l->head == NULL) {
		return;
	}

	l->length -= 1;
	if (l->length == 0) {
		free(l->head);
		l->head = l->tail = NULL;
	}

	struct llist_node *p = l->head;
	l->head = p->nxt;
	free(p);
}
