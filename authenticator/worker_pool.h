#include "queue.h"

#include <pthread.h>

typedef struct _worker_pool {
    int num_threads;
    void (*do_work)(void *);
    pthread_t *threads;
    queue *work_queue;
    volatile char stop;
    pthread_mutex_t queue_mutex;
} worker_pool;

worker_pool *pool_new(int num_threads, void (*)(void *));
void pool_del(worker_pool *pool);

void pool_start(worker_pool *pool);
void pool_stop(worker_pool *pool);

void pool_add_work(worker_pool *pool, void *job);
