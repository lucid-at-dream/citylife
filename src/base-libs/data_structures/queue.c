#include "queue.h"

#include <stdlib.h>

queue_item *item_new(void *content) {
    queue_item *item = (queue_item *)calloc(1, sizeof(queue_item));
    item->content = content;
    return item;
}

queue *queue_new() {
    queue *q = (queue *)calloc(1, sizeof(queue));
    return q;
}

void queue_del(queue *q) {
    while (q->last != NULL) {
        queue_item *tmp = q->last;
        q->last = q->last->prev;
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

void *queue_pop(queue *q) {
    queue_item *item = q->head;

    if (item == NULL) {
        return NULL;
    }

    q->head = q->head->next;
    if (q->head != NULL) {
        q->head->prev = NULL;
    } else {
        q->last = NULL;
    }

    void *content = item->content;
    free(item);
    q->size--;
    return content;
}
