// This code follows Prof. Bernstein’s original code, which is released into the public domain. 
// It source and a detailed explaination can be found at http://github.com/agl/critbit.

#include "util/critbit.h"
#include "util/misc.h"
#include <string.h>
#include <stdlib.h>

// critbit tree internal node
struct critbit_intern {
	uint8_t type;	// used for indicating the is a internal node, will be set to 0
	void *child[2];	// child nodes
	size_t cbit;	// critcal bit: the first bit from begining that differs. 
};

// Initializes a cirbit tree.
void critbit_init(struct critbit_tree *self) {
	self->rt = NULL;
}

// Walk the tree to find a best member matching the expected key.
// If wherep is not NULL, sets parameter wherep, which is the father pointer to the result.
// Internal function only: called by critbit_get() and critbit_insert()
// Does not accept empty string as a key.
// No guarantee for the returned node is the wanted node, but if not
// we can conclude the wanted node does not exist.
// Never returns NULL. Does not handle the corner case of a empty tree, i.e.
// the caller must ensure self->rt is not NULL.
static struct critbit_node* critbit_walk(struct critbit_tree *self, const char *key, void ***wherep) {
	const size_t len = strlen(key);

	if (wherep) {
		*wherep = NULL;
	}

	// Note that the pointer will never be NULL as long as self->rt is not NULL.
	// If an internal node had a NULL pointer then the tree would be 
	// invalid - that internal node should be removed.
	struct critbit_intern *p = self->rt;
	while (p->type == 0) {
		const uint8_t c = ((p->cbit >> 3) < len) ? key[p->cbit >> 3] : 0;
		const uint8_t bit_mask = ~(1 << (p->cbit & 7)); // here all the bits but the critical are 1.
		const int dir = (1 + (c | bit_mask)) >> 8;
		if (wherep) {
			*wherep = p->child + dir;
		}
		p = p->child[dir];
	}

	return ((void*)p);
}

// Get a node in tree by its key.
// Returns NULL if the wanted node does not exists.
// Does not accept empty string as a key.
struct critbit_node* critbit_get(struct critbit_tree *self, const char *key) {
	if (!key || key[0] == '\0') {
		return (NULL);
	}

	if (!self->rt) {
		return (NULL);
	}

	struct critbit_node *p = critbit_walk(self, key, NULL);
	// We can now only conclude that certain bits of node p are shared with wanted key.
	// We still need to test the best match to make sure that it’s correct. If the test fails,
	// however, we can conclude that the string is not in the tree.
	if (strequal(key, p->key)) {
		return (p);
	} else {
		return (NULL);
	}
}

// Inserts a value into the tree.
// If the tree already contains a node with the same key, that node
// will be replaced and returned, otherwise, this function returns NULL.
// Memory leaks if the returned node was not handled properly. 
// Does not allow empty string as a key.
struct critbit_node* critbit_insert(struct critbit_tree *self, struct critbit_node *x) {
	// Mark x as a external node.
	x->type = -1;

	if (!self->rt) {
		self->rt = x;
		return (NULL);
	}

	void **wherep;
	struct critbit_node *p = critbit_walk(self, x->key, &wherep);

	// Now that we have found the best match for the new element in the tree,
	// we need to check to see where the new element differs from that element.
	// We will be calculating the differing byte and the differing bits separately for now
	// differbit is the XOR of the differing byte. They will analyzed and merged later.
	char *u = x->key, *v = p->key;
	const size_t ulen = strlen(u), vlen = strlen(v);

	size_t differbyte;
	int differbit;
	for (differbyte = 0; differbyte < ulen && differbyte < vlen; ++differbyte) {
		if (v[differbyte] != u[differbyte]) {
			differbit = v[differbyte] ^ u[differbyte];
			goto DIFFER_FOUND;
		}
	}

	if (v[differbyte] != 0) {
		differbit = v[differbyte];
		goto DIFFER_FOUND;
	}

	if (u[differbyte] != 0) {
		differbit = u[differbyte];
		goto DIFFER_FOUND;
	}

	if (wherep) {
		*wherep = x;
	} else {
		self->rt = x;
	}
	return (p);

DIFFER_FOUND:
	size_t new_critbit = differbyte << 3;
	while ((differbit & 1) == 0) {
		new_critbit += 1;
		differbit >>= 1;
	}

	const uint8_t c = (differbyte < vlen) ? v[differbyte] : 0;
	const uint8_t bit_mask = ~(1 << (new_critbit & 7)); // here all the bits but the critical are 1.
	const int new_dir = (1 + (c | bit_mask)) >> 8;

	struct critbit_intern *new_node = try_malloc(sizeof(struct critbit_intern), __FUNCTION__);
	new_node->type = 0;
	new_node->cbit = new_critbit;
	new_node->child[1 - new_dir] = x;

	// Now we will insert new_node into the tree.
	// Inserting the new node in the tree involves walking the tree from
	// the root to find the correct position to insert at.
	// We keep track of the pointer to be updated (to point to the new internal node),
	// and once the walk has finished, we can update that pointer.
	void **whereq = &self->rt;
	while (1) {
		struct critbit_intern *q = *whereq;
		if (q->type) {
			break;
		}

		if (q->cbit > new_node->cbit) {
			break;
		}

		const char c = ((q->cbit >> 3) < ulen) ? u[q->cbit >> 3] : 0;
		const uint8_t bit_mask = ~(1 << (q->cbit & 7)); // here all the bits but the critical are 1.
		const int dir = (1 + (c | bit_mask)) >> 8;
		whereq = q->child + dir;
	}

	new_node->child[new_dir] = *whereq;
	*whereq = new_node;
	return (NULL);
}

// Removes and returns the node with given key.
// If the wanted node was not found, returns NULL.
struct critbit_node *critbit_erase(struct critbit_tree *self, const char *key) {
	if (!self->rt) {
		return (NULL);
	}

	// Walking the tree to find the best match is almost the same as the critbit_walk().
	// The only exception is that we keep track of the last jump to an internal node in whereq.
	// Actually, we keep track of a pointer to the last pointer that got us to an internal node.
	int dir;
	void **wherep = &self->rt;
	void **whereq = NULL;
	struct critbit_intern *p = self->rt, *q;
	const size_t len = strlen(key);

	while (p->type == 0) {
		whereq = wherep;
		q = p;
		const uint8_t c = ((p->cbit >> 3) < len) ? key[p->cbit >> 3] : 0;
		const uint8_t bit_mask = ~(1 << (p->cbit & 7)); // here all the bits but the critical are 1.
		dir = (1 + (c | bit_mask)) >> 8;
		wherep = q->child + dir;
		p = q->child[dir];
	}

	if (!strequal(key, ((struct critbit_node*)p)->key)) {
		return (NULL);
	}

	if (!whereq) {
		self->rt = NULL;
		return ((void*)p);
	}

	*whereq = q->child[1 - dir];
	free(q);
	return ((void*)p);
}
