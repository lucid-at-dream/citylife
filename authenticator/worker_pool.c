#include "worker_pool.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *worker_loop(void *args) {
    worker_pool *pool = (worker_pool *)args;
    while(!pool->stop)
    {
        while(pool->work_queue->size > 0)
        {
            pthread_mutex_lock(&(pool->queue_mutex));
            void *job = queue_pop(pool->work_queue);
            pthread_mutex_unlock(&(pool->queue_mutex));

            if (job != NULL) {
                pool->do_work(job);
            }
        }
    }
    return 0;
}

void pool_add_work(worker_pool *pool, void *job) {
    
    // No more work is done if the pool has been signaled to stop working.
    if (pool->stop) {
        return;
    }

    pthread_mutex_lock(&(pool->queue_mutex));
    queue_add(pool->work_queue, job);

    // pthread_cond_signal(&condB);
    pthread_mutex_unlock(&(pool->queue_mutex));
}

worker_pool *pool_new(int num_threads, void (*do_work)(void *)) {
    worker_pool *pool = (worker_pool *)calloc(1, sizeof(worker_pool));
    pool->threads = (pthread_t *)calloc(num_threads, sizeof(pthread_t));
    pool->num_threads = num_threads;
    pool->do_work = do_work;
    pool->work_queue = queue_new();
    pool->stop = 0;
    pthread_mutex_init(&(pool->queue_mutex), NULL);
    return pool;
}

void pool_start(worker_pool *pool) {
    for (int i = 0; i < pool->num_threads; i++) {
        pthread_create(&(pool->threads[i]), NULL, worker_loop, pool);
    }
}

void pool_stop(worker_pool *pool) {
    // Flag to stop
    pool->stop = 1;

    // Wait for completion
    for (int i = 0; i < pool->num_threads; i++) {
        pthread_join(pool->threads[i], NULL);
    }
}

void pool_del(worker_pool *pool) {
    pthread_mutex_destroy(&(pool->queue_mutex));
    queue_del(pool->work_queue);
    free(pool->threads);
    free(pool);
}
