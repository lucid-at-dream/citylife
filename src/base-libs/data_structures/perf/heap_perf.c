#include "perftest.h"

#include "heap.h"

#include <stdlib.h>

int int_compare(const void *a, const void *b) {
    int int_a = (int)a;
    int int_b = (int)b;

    if (int_a < int_b) {
        return -1;
    } else if (int_a > int_b) {
        return 1;
    }
    return 0;
}

PERF_TEST(test_heap_sort_of_random_elements, {
    int total_elements = 500;

    heap *h = heap_new(int_compare);

    srandom(0);

    for (int i = 0; i < total_elements; i++) {
        int r = (int)random();
        h = heap_push(h, r);
    }

    for (int i = 0; i < total_elements; i++) {
        heap_pop(h);
    }

    heap_destroy(h);
})

// TODO: For every write operation of the heap, assert the invariants.

PERF_TEST_SUITE(RUN_PERF_TEST("Test heap sorting several random elements.", &test_heap_sort_of_random_elements), )
