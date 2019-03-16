#include "queue.h"

typedef struct _worker_pool {
    int num_threads;
    void *(*do_work)(void *);
    int *threads;
    queue *work_queue;
} worker_pool;

worker_pool *worker_pool_new(int num_threads, void *(*)(void *));
void worker_pool_start(worker_pool *pool);
void worker_pool_stop(worker_pool *pool);

void pool_add_work(worker_pool *pool, void *job);
