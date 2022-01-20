#include "heap.h"

#include <stdlib.h>

heap *heap_new() {
    return (heap *)calloc(1, sizeof(heap));
}

void heap_push(heap *h, void *item) {
    h->item = item;
}

void *heap_pop(heap *h) {
    void *tmp = h->item;
    h->item = NULL;
    return tmp;
}

void *heap_peek(heap *h) {
    return h->item;
}

void heap_destroy(heap *h) {
    free(h);
}