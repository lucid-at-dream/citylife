
typedef struct _worker_pool {
    int num_threads;
    char *(do_work)(char *);
} worker_pool;

worker_pool *worker_pool_new(int num_threads, char *(do_work)(char *));
void worker_pool_start(worker_pool *pool);
void worker_pool_stop(worker_pool *pool);
