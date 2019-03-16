
typedef struct _worker_pool {
    int num_threads;
    void *(do_work)(void *);
    int *threads;
    queue work_queue;
} worker_pool;

worker_pool *worker_pool_new(int num_threads, void *(do_work)(void *)) {
    worker_pool pool = (worker_pool *)calloc(1, sizeof(worker_pool));
    threads = (int *)calloc(num_threads, sizeof(int));
    pool->num_threads = num_threads;
    pool->do_work = do_work;
}

worker_pool_start(worker_pool *pool);

worker_pool_stop(worker_pool *pool);
