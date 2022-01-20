#pragma once

#include "queue.h"

typedef struct _heap {
    
    // Number of nodes below this one
    int size;

    // Number of children
    int degree;

    // is active or passive?
    char is_active;

    // Number of active children (if the node is active)
    int rank;

    // ??? Total loss of an heap is the sum of the loss over all active nodes
    unsigned loss;

    void *item;

    struct _heap *children;
} heap;

typedef struct _heap_node {
    int degree;
} heap_node;

// All nodes except the root are kept in this queue
queue *Q;

heap *heap_new();

void heap_destroy(heap *h);

void heap_insert(heap *h, void *value);

void heap_meld(heap *h1, heap *h2);

void *heap_find_min(heap *h);

void *heap_delete_min(heap *h);

void heap_delete(heap *h, void *e);

void heap_decrease_key(heap *h, void *e, void *new_key);

