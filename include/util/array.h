#ifndef ACC_UTIL_ARRAY_H
#define ACC_UTIL_ARRAY_H

struct array {
	int length;
	int cap;
	void **begin;
};

void array_init(struct array *a);
void array_pushback(struct array *a, void *val);
void array_free(struct array *a);
void* array_get(struct array *a, int index);
void array_set(struct array *a, int index, void *val);
void* array_popback(struct array *a);

#endif
