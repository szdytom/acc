#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util/array.h"
#include "fatals.h"

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
		a->cap = 1;
	} else if (a->cap <= 256) {
		a->cap *= 8;
	} else {
		a->cap *= 1.7;
	}

	a->begin = realloc(a->begin, a->cap);
	if (a->begin == NULL) {
		fail_malloc(__FUNCTION__);
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

void* array_popback(struct array *a) {
	if (a->length == 0) {
		return (NULL);
	}

	a->length -= 1;
	return (a->begin[a->length]);
}