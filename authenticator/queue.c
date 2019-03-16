#include "queue.h"

typedef struct _queue_item {
    void *content;
    struct _queue_item* prev;
    struct _queue_item* next;
} queue_item;

typedef struct _queue {
    int size;
    queue_item *head;
    queue_item *last;
} queue;

queue_item* item_new(void *content) {
    queue_item *item = (queue_item *)calloc(1, sizeof(queue_item));
    item->content = content;
    return item;
}

queue *queue_new() {
    queue *q = (queue *)calloc(1, sizeof(queue));
    return q;
}

queue *queue_del(queue *q) {
    queue_item *to_free = q->last;
    while (to_free->prev != NULL) {
        queue_item *tmp = to_free;
        to_free = to_free->prev;
        free(tmp);
    }
    free(q);
}

void queue_add(queue *q, void *item) {
    queue_item *item_to_add = item_new(item);
    
    if (q->size == 0) {
        q->head = q->last = item_to_add;
        item_to_add->next = item_to_add->prev = NULL;
    } else {
        q->last->next = item_to_add;
        item_to_add->prev = q->last;
        item_to_add->next = NULL;
        q->last = item_to_add;
    }

    q->size++;
    return;
}

void *pop(queue *q) {
    queue_item *head = q->head;
    q->head = q->head->next;
    q->head->prev = NULL;
    void *content = head->content;
    free(head);
    return content;
}
