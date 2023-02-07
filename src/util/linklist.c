#include <stdlib.h>
#include <stdio.h>
#include "fatals.h"
#include "util/linklist.h"

// Create a linklist node using given value.
struct llist_node* llist_createnode(void *val) {
	struct llist_node *res = malloc(sizeof(struct llist_node));
	if (res == NULL) {
		fail_malloc(__FUNCTION__);
	}
	res->nxt = NULL;
	res->val = val;
	return (res);
}

// Appends an element in the linklist. 
void llist_pushback(struct linklist *l, void *val) {
	l->length += 1;
	if (!l->tail) {
		l->head = l->tail = llist_createnode(val);
		return;
	}
	l->tail->nxt = llist_createnode(val);
	l->tail = l->tail->nxt;
}

// A variant of pushback
// Only does pushback if _val_ is not null.
void llist_pushback_notnull(struct linklist *l, void *val) {
	if (val) {
		llist_pushback(l, val);
	}
}

// Returns the _index_ thh element.
void* llist_get(struct linklist *l, int index) {
	if (index >= l->length) {
		fprintf(stderr, "linklist out of range.\n");
		abort();
	}

	struct llist_node *p = l->head;
	for (int i = 0; i < index; ++i) {
		p = p->nxt;
	}
	return (p->val);
}

// Modify the _index_ thh element.
void llist_set(struct linklist *l, int index, void *val) {
	if (index >= l->length) {
		fprintf(stderr, "linklist out of range.\n");
		abort();
	}

	struct llist_node *p = l->head;
	for (int i = 0; i < index; ++i) {
		p = p->nxt;
	}
	p->val = val;
}

// Init a empty linklist.
void llist_init(struct linklist *l) {
	l->length = 0;
	l->head = NULL;
	l->tail = NULL;
}

// Frees the linklist.
// Caller must make sure all elements in the linklist has already been freed. 
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

// Frees the linklist.
// Callee will free all elements in the link list.
void llist_free_full(struct linklist *l) {
	struct llist_node *p = l->head;
	struct llist_node *nxt;
	while (p) {
		nxt = p->nxt;
		if (p->val) {
			free(p->val);
		}
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

// Pop the first element of the link list
// Return the first element.
void* llist_popfront(struct linklist *l) {
	if (l->head == NULL) {
		return (NULL);
	}

	l->length -= 1;
	void *res = l->head->val;
	if (l->length == 0) {
		free(l->head);
		l->head = l->tail = NULL;
		return (res);
	}

	struct llist_node *p = l->head;
	l->head = p->nxt;
	free(p);
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
	void *res = q->val;
	free(q);
	return res;
}
