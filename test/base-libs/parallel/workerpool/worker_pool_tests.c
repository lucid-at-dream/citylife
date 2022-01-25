#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "worker_pool.h"
#include "test.h"
#include "assert.h"

pthread_mutex_t global_sum_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile int global_sum;
const int sleep_time_micros = 250;

void summation(void *value) {
    int v = *((int *)value);

    // Sleep to simulate a thread blocked for IO
    usleep(sleep_time_micros);

    pthread_mutex_lock(&global_sum_mutex);
    global_sum += v;
    pthread_mutex_unlock(&global_sum_mutex);
}

unsigned long get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return 1000000 * tv.tv_sec + tv.tv_usec;
}

char test_worker_pool_parallel_summation() {
    int num_threads = 4;
    int sum_limit = 2000;
    unsigned long begin, end;

    // Do the work single threadedly
    printf("Single thread starting\n");
    begin = get_time();
    int expected_sum = 0;
    for (int value = 1; value < sum_limit; value++) {
        // Sleep to simulate a thread blocked for IO
        usleep(sleep_time_micros);
        expected_sum += value;
    }
    end = get_time();
    unsigned long elapsed_time_single_thread = end - begin;

    // Do the same work in parallel
    printf("Multi thread starting\n");
    begin = get_time();
    global_sum = 0;

    worker_pool *pool = pool_new(num_threads, summation);
    pool_start(pool);

    int *values[sum_limit];

    for (int value = 1; value < sum_limit; value++) {
        int *copy_value = (int *)malloc(sizeof(int));
        *copy_value = value;
        pool_add_work(pool, copy_value);
        values[value - 1] = copy_value;
    }

    printf("Added all work to pool\n");

    pool_await_empty_queue(pool);
    pool_stop(pool);
    pool_del(pool);
    end = get_time();
    unsigned long elapsed_time_multi_thread = end - begin;

    // Free allocated memory
    printf("Freeing allocated resources\n");
    for (int i = 0; i < sum_limit - 1; i++) {
        free(values[i]);
    }

    int assertion_error =
            assert_int_less_than("Execution time in parallel should be less than single threadedly", elapsed_time_multi_thread, elapsed_time_single_thread);
    if (assertion_error) {
        return 1;
    }

    assertion_error = assert_int_equals("The sum from the thread pool equals the sum from the single threaded summation", global_sum, expected_sum);
    if (assertion_error) {
        return 1;
    }

    return 0;
}

test test_suite[] = { { "Test summing some numbers in parallel", test_worker_pool_parallel_summation } };

int main(int argc, char **argv) {
    suite_report report = run_test_suite(test_suite, sizeof(test_suite) / sizeof(test));

    if (report.failures > 0) {
        return -1;
    }

    return 0;
}
