
typedef struct _queue_item {
    void *content;
    struct _queue_item* next; 
} queue_item;

typedef struct _queue {
    int size;
    queue_item *head;
} queue;

void queue_add(queue *, void *)

void *pop(queue *, void *)
