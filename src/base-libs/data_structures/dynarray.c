#include "dynarray.h"

#include <stdlib.h>

dynarray *dynarray_new() {
    return (dynarray *)calloc(1, sizeof(dynarray));
}

dynarray *dynarray_destroy(dynarray *ar) {
    if (ar->data != NULL) {
        free(ar->data);
    }
    free(ar);
}

void dynarray_add(dynarray *ar, void *el) {
    if (ar->size >= ar->capacity) {
        int increment = (int)((ar->capacity * 3) / 2.0 + 1);
        ar->data = realloc(ar->data, sizeof(void *) * (ar->capacity + increment)); // TODO: What if no more memory?
        ar->capacity += increment;
    }
    ar->data[ar->size] = el;
    ar->size += 1;
}

void dynarray_del(dynarray *ar, int idx) {
    if (idx >= ar->size) {
        return; // TODO: Some sort of error handling would be nice
    }

    ar->data[idx] = NULL;
    for (int i = idx; i < ar->size - 1; i++) {
        ar->data[i] = ar->data[i + 1];
    }

    ar->size -= 1;

    return NULL;
}

void *dynarray_get(dynarray *ar, int idx) {
    if (idx >= ar->size) {
        return NULL; // TODO: Some sort of error handling would be nice.
    }
    return ar->data[idx];
}

void dynarray_set(dynarray *ar, int idx, void *el) {
    if (idx >= ar->size) {
        return; // TODO: Some sort of error handling would be nice.
    }
    ar->data[idx] = el;
}
