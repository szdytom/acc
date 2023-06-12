#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "fatals.h"
#include "util/linklist.h"

// Appends an element in the linklist. 
void llist_pushback(struct linklist *l, void *val) {
	struct llist_node *x = (struct llist_node*)val;
	x->nxt = NULL;
	l->length += 1;
	if (!l->tail) {
		l->head = l->tail = val;
		return;
	}

	l->tail->nxt = x;
	l->tail = x;
}

// A variant of pushback
// Only does pushback if _val_ is not null.
void llist_pushback_notnull(struct linklist *l, void *val) {
	if (val) {
		llist_pushback(l, val);
	}
}

// Returns the _index_ th element.
void* llist_get(struct linklist *l, int index) {
	if (index >= l->length) {
		fprintf(stderr, "linklist out of range.\n");
		abort();
	}

	struct llist_node *p = l->head;
	for (int i = 0; i < index; ++i) {
		p = p->nxt;
	}
	return (p);
}

// Check if the given linklist is empty
bool llist_isempty(struct linklist *l) {
	return (l->length == 0);
}

// Init a empty linklist.
void llist_init(struct linklist *l) {
	l->length = 0;
	l->head = NULL;
	l->tail = NULL;
}

// Frees all elements in the link list.
// Memory leaks if linklist element is still containing some pointer.
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

// Insert _val_ into the linklist as the _index_ th element.
// If _index_ is too large, pushback only!
void llist_insert(struct linklist *l, int index, void *val) {
	if (index >= l->length) {
		llist_pushback(l, val);
		return;
	}

	struct llist_node *x = (struct llist_node*)val;
	l->length += 1;
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

// Pop the first element of the link list
// Return the first element.
void* llist_popfront(struct linklist *l) {
	if (l->head == NULL) {
		return (NULL);
	}

	l->length -= 1;
	struct llist_node *res = l->head;
	l->head = res->nxt;
	res->nxt = NULL;
	if (l->length == 0) {
		l->head = l->tail = NULL;
	}

	return (res);
}

// Removes the _index_ th element from the linklist.
// Returns the removed value.
void* llist_remove(struct linklist *l, int index) {
	if (index >= l->length) {
		return (NULL);
	}

	if (index == 0) {
		return (llist_popfront(l));
	}

	l->length -= 1;
	struct llist_node *p = l->head;
	for (int i = 0; i < index - 2; ++i) {
		p = p->nxt;
	}
	struct llist_node *q = p->nxt;
	p->nxt = q->nxt;
	q->nxt = NULL;
	return (q);
}
