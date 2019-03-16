#include "worker_pool.h"

#include <stdlib.h>

void pool_add_work(worker_pool *pool, void *job) {
    queue_add(pool->work_queue, job);
}

worker_pool *pool_new(int num_threads, void *(*do_work)(void *)) {
    worker_pool *pool = (worker_pool *)calloc(1, sizeof(worker_pool));
    pool->threads = (int *)calloc(num_threads, sizeof(int));
    pool->num_threads = num_threads;
    pool->do_work = do_work;
    pool->work_queue = queue_new();
    return pool;
}

void pool_start(worker_pool *pool) {

}

void pool_stop(worker_pool *pool) {
    
}
