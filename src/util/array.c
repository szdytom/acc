#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util/array.h"

void array_init(struct array *a) {
	a->length = 0;
	a->cap = 0;
	a->begin = NULL;
}

void array_free(struct array *a) {
	free(a->begin);
	a->begin = NULL;
	a->length = 0;
	a->cap = 0;
}

static void array_enlarge(struct array *a) {
	if (a->cap == 0) {
		a->cap = 128;
	} else {
		a->cap *= 2;
	}

	void **old = a->begin;
	a->begin = malloc(sizeof(void*) * a->cap);
	if (old) {
		memcpy(a->begin, old, a->length * sizeof(void*));
	}
}

void array_pushback(struct array *a, void *val) {
	if (a->length == a->cap) {
		array_enlarge(a);
	}

	a->begin[a->length] = val;
	a->length += 1;
}

void* array_get(struct array *a, int index) {
	if (index >= a->length) {
		fprintf(stderr, "%s: out of range.\n", __FUNCTION__);
		abort();
	}

	return (a->begin[index]);
}

void array_set(struct array *a, int index, void *val) {
	if (index >= a->length) {
		fprintf(stderr, "%s: out of range.\n", __FUNCTION__);
		abort();
	}

	a->begin[index] = val;
}
