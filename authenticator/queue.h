
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

// Create and delete a queue
void queue_del(queue *q);
queue *queue_new();

// Add/Pop to/from queue
void queue_add(queue *, void *);
void *queue_pop(queue *q);
