// This file implements the crit-bit tree, which is a variant of trie.
// See https://github.com/agl/critbit/ for algorithm details.

#ifndef ACC_UTIL_CRITBIT_H
#define ACC_UTIL_CRITBIT_H

#include <stdint.h>
#include <stdbool.h>

// crit-bit tree external node
struct critbit_node {
	uint8_t type;	// used for indicating this is a external node, this field will always be set to -1
	char *key;	// string key for this value
};

// crit-bit tree
struct critbit_tree {
	void *rt;	// root node
};

// Initializes a cirbit tree.
void critbit_init(struct critbit_tree *self);

// Get a node in tree by its key.
// Returns NULL if the wanted node does not exists.
// Does not accept empty string as a key.
struct critbit_node* critbit_get(struct critbit_tree *self, const char *key);

// Inserts a value into the tree.
// If the tree already contains a node with the same key, that node
// will be replaced and returned, otherwise, this function returns NULL.
// Memory leaks if the returned node was not handled properly. 
// Does not allow empty string as a key.
struct critbit_node* critbit_insert(struct critbit_tree *self, struct critbit_node *x);

// Removes and returns the node with given key.
// If the wanted node was not found, returns NULL.
struct critbit_node *critbit_erase(struct critbit_tree *self, const char *key);

#endif