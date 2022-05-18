#pragma once

typedef struct _queue_item
{
    void *content;
    volatile struct _queue_item *prev;
    volatile struct _queue_item *next;
} queue_item;

typedef struct _queue
{
    volatile int size;
    volatile queue_item *head;
    volatile queue_item *last;
} queue;

// Create and delete a queue
void queue_del(queue *q);
queue *queue_new();

// Add/Pop to/from queue
queue_item *queue_add(queue *, void *);
void *queue_pop(queue *q);

// Merge other queue into q
void queue_merge_into(queue *q, queue *other);

// Delete an element from the queue in O(1)
void queue_remove_node(queue *q, queue_item *n);
