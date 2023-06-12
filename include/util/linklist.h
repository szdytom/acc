#ifndef ACC_UTIL_LINKLIST_H
#define ACC_UTIL_LINKLIST_H

struct llist_node {
	struct llist_node *nxt;
};

struct linklist {
	int length;
	struct llist_node *head;
	struct llist_node *tail;
};

void llist_init(struct linklist *l);
void llist_free(struct linklist *l);
void llist_free_full(struct linklist *l);

void llist_pushback(struct linklist *l, void *val);
void llist_pushback_notnull(struct linklist *l, void *val);

void* llist_get(struct linklist *l, int x);
void llist_set(struct linklist *l, int x, void *val);

void llist_insert(struct linklist *l, int x, void *val);
void* llist_popfront(struct linklist *l);
void* llist_remove(struct linklist *l, int index);

#endif
