#ifndef ACC_UTIL_LINKLIST_H
#define ACC_UTIL_LINKLIST_H

struct llist_node {
	void *val;
	struct llist_node *nxt;
};

struct linklist {
	int length;
	struct llist_node *head;
	struct llist_node *tail;
};

struct llist_node* llist_createnode(void *val);
void llist_pushback(struct linklist *l, void *val);
void llist_pushback_notnull(struct linklist *l, void *val);
void* llist_get(struct linklist *l, int x);
void llist_set(struct linklist *l, int x, void *val);
void llist_init(struct linklist *l);
void llist_free(struct linklist *l);
void llist_insert(struct linklist *l, int x, void *val);
void llist_popfront(struct linklist *l);

#endif
