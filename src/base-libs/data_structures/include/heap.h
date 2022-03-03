#pragma once

#include "queue.h"
#include "list.h"

typedef struct _heap heap;
typedef struct _heap_node heap_node;

heap *heap_new(int (*compare)(const void *a, const void *b));

void heap_destroy(heap *h);

heap *heap_push(heap *h, void *item, heap_node **node);

heap *heap_meld(heap *h1, heap *h2);

void *heap_find_min(heap *h);

void *heap_delete_min(heap *h);

void heap_delete(heap *h, void *e);

void heap_decrease_key(heap *h, heap_node *x, void *new_item);
