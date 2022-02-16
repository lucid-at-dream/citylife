#include "test.h"
#include "assert.h"

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

TEST_CASE(test_destroy_empty_heap, {
    heap *h = heap_new(int_compare);
    heap_destroy(h);
})

TEST_CASE(test_heap_push_one_then_peek_assert_equals, {
    heap *h = heap_new(int_compare);

    h = heap_push(h, 10);

    ASSERT_INT_EQUALS("Peek of heap with 1 item is the item.", (int)heap_peek(h), 10);

    heap_destroy(h);
})

TEST_CASE(test_heap_push_multiple_elements_in_reverse_order_assert_min_peek, {
    heap *h = heap_new(int_compare);

    for (int i = 10; i >= 0; i--) {
        h = heap_push(h, i);
    }

    ASSERT_INT_EQUALS("Peek of heap returns minimum value in heap.", (int)heap_peek(h), 0);

    heap_destroy(h);
})

TEST_CASE(test_heap_push_multiple_elements_in_random_order_assert_min_peek, {
    heap *h = heap_new(int_compare);

    h = heap_push(h, 1);
    h = heap_push(h, 4);
    h = heap_push(h, 3);
    h = heap_push(h, 0);
    h = heap_push(h, 2);

    ASSERT_INT_EQUALS("Peek of heap returns minimum value in heap.", (int)heap_peek(h), 0);

    heap_destroy(h);
})

TEST_CASE(test_heap_push_multiple_elements_assert_min_popped, {
    heap *h = heap_new(int_compare);

    for (int i = 0; i < 10; i++) {
        h = heap_push(h, i);
    }

    for (int i = 0; i < 10; i++) {
        int peeked_value = (int)heap_peek(h);
        ASSERT_INT_EQUALS("Peek of heap returns minimum value in heap.", peeked_value, i);

        int popped = (int)heap_pop(h);
        ASSERT_INT_EQUALS("Pop of heap with returns the minimum value in the heap.", popped, i);
    }

    heap_destroy(h);
})

TEST_CASE(test_heap_sort_of_random_elements, {
    int total_elements = 1000;
    
    heap *h = heap_new(int_compare);

    srandom(0);

    for (int i = 0; i < total_elements; i++) {
        int r = (int)random();
        h = heap_push(h, r);
    }

    long int previous_value = -__INT_MAX__;

    for (int i = 0; i < total_elements; i++) {
        int popped = (int)heap_pop(h);
        ASSERT_TRUE("Heap sort gets elements to increasing order", popped >= previous_value);
        previous_value = popped;
    }

    heap_destroy(h);
})

// TODO: For every write operation of the heap, assert the invariants.

TEST_SUITE(
    RUN_TEST("Test destroying an empty heap.", &test_destroy_empty_heap),
    RUN_TEST("Test push pop of an item in a heap works.", &test_heap_push_one_then_peek_assert_equals),
    RUN_TEST("Test push multiple items in reverse order and check min.", &test_heap_push_multiple_elements_in_reverse_order_assert_min_peek),
    RUN_TEST("Test push multiple items in random order and check min.", &test_heap_push_multiple_elements_in_random_order_assert_min_peek),
    RUN_TEST("Test push multiple items in order and pop min all of them.", &test_heap_push_multiple_elements_assert_min_popped),
    RUN_TEST("Test heap sorting several random elements.", &test_heap_sort_of_random_elements),
)
