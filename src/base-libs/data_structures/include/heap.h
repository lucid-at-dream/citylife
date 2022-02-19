#pragma once

#include "queue.h"
#include "list.h"

typedef struct _heap_node {
    // Number of nodes below this one
    int size;

    // is active or passive?
    char is_active; // TODO: Make this a pointer to an active record

    // Number of active children (if the node is active)
    int rank; // TODO: Should this be a pointer to the rank list?

    // ??? Total loss of an heap is the sum of the loss over all active nodes
    unsigned loss; // TODO: Is this being correctly updated?

    // The item being stored in this heap node
    void *item;

    // The parent of the node
    struct _heap_node *parent;

    queue_item *position_in_q;

    // List of the children of this node
    list *children;
} heap_node;

typedef struct _heap {
    // Total number of nodes in the heap
    int size;

    // Root node of the heap
    heap_node *root;

    // Returns -1 if a < b, 1 if a > b and 0 if they're equal
    int (*compare)(const void *a, const void *b);

    // All nodes except the root are kept in this queue
    queue *Q;

} heap;

heap *heap_new(int (*compare)(const void *a, const void *b));

void heap_destroy(heap *h);

heap *heap_push(heap *h, void *item);

heap *heap_meld(heap *h1, heap *h2);

void *heap_find_min(heap *h);

void *heap_delete_min(heap *h);

void heap_delete(heap *h, void *e);

void heap_decrease_key(heap *h, heap_node *x, void *new_item);
