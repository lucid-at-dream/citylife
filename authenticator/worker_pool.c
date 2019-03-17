#include "worker_pool.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *worker_loop(void *args) {
    worker_pool *pool = (worker_pool *)args;
    while(!pool->stop)
    {
        // Wait for the job
        pthread_mutex_lock(&(pool->queue_mutex));
        while (!pool->stop && pool->work_queue->size <= 0) { // If there's no work and we shouldn't stop: wait.
            pthread_cond_wait(&(pool->await_work_cond), &(pool->queue_mutex));
        }

        // Get the job
        void *job = queue_pop(pool->work_queue);

        // Signal changes to the queue
        pthread_cond_signal(&(pool->await_finish_cond));
        pthread_mutex_unlock(&(pool->queue_mutex));

        // Do the job
        if (job != NULL) {
            pool->do_work(job);
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
    pthread_cond_signal(&(pool->await_work_cond)); // Let the worker threads know there's a job
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
    pthread_cond_init(&(pool->await_work_cond), NULL);
    pthread_cond_init(&(pool->await_finish_cond), NULL);
    return pool;
}

void pool_start(worker_pool *pool) {
    for (int i = 0; i < pool->num_threads; i++) {
        pthread_create(&(pool->threads[i]), NULL, worker_loop, pool);
    }
}

void pool_stop(worker_pool *pool) {
    // Flag to stop
    pthread_mutex_lock(&(pool->queue_mutex));
    pool->stop = 1;

    for (int i = 0; i < pool->num_threads; i++) {
        pthread_cond_signal(&(pool->await_work_cond)); // Let the worker threads know there's something to do (stop)
    }
    
    pthread_mutex_unlock(&(pool->queue_mutex));

    // Wait for completion
    for (int i = 0; i < pool->num_threads; i++) {
        pthread_join(pool->threads[i], NULL);
    }
}

void pool_await_empty_queue(worker_pool *pool) {
    pthread_mutex_lock(&(pool->queue_mutex));
    while(pool->work_queue->size > 0) {
        pthread_cond_wait(&(pool->await_finish_cond), &(pool->queue_mutex));
    }
    pthread_mutex_unlock(&(pool->queue_mutex));
}

void pool_del(worker_pool *pool) {
    pthread_mutex_destroy(&(pool->queue_mutex));
    queue_del(pool->work_queue);
    free(pool->threads);
    free(pool);
}
