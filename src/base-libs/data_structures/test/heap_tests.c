#include "assert.h"
#include "test.h"

#include "heap.h"
#include "heap_checks.h"

#include <math.h>
#include <stdlib.h>

int int_compare(const void *a, const void *b)
{
    int int_a = (int)a;
    int int_b = (int)b;

    if (int_a < int_b)
    {
        return -1;
    }
    else if (int_a > int_b)
    {
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

    heap_node *node;
    h = heap_push(h, 10, &node);

    ASSERT_INT_EQUALS("Peek of heap with 1 item is the item.", (int)heap_peek(h), 10);

    heap_destroy(h);
})

TEST_CASE(test_heap_push_multiple_elements_in_reverse_order_assert_min_peek, {
    heap *h = heap_new(int_compare);

    for (int i = 10; i >= 0; i--)
    {
        heap_node *node;
        h = heap_push(h, i, &node);
        ASSERT_TRUE("All invariants are satisfied", validate_invariants(h) == 0);
    }

    ASSERT_INT_EQUALS("Peek of heap returns minimum value in heap.", (int)heap_peek(h), 0);

    heap_destroy(h);
})

TEST_CASE(test_heap_push_multiple_elements_in_random_order_assert_min_peek, {
    heap *h = heap_new(int_compare);

    heap_node *node;
    h = heap_push(h, 1, &node);
    ASSERT_TRUE("All invariants are satisfied", validate_invariants(h) == 0);

    h = heap_push(h, 4, &node);
    ASSERT_TRUE("All invariants are satisfied", validate_invariants(h) == 0);

    h = heap_push(h, 3, &node);
    ASSERT_TRUE("All invariants are satisfied", validate_invariants(h) == 0);

    h = heap_push(h, 0, &node);
    ASSERT_TRUE("All invariants are satisfied", validate_invariants(h) == 0);

    h = heap_push(h, 2, &node);
    ASSERT_TRUE("All invariants are satisfied", validate_invariants(h) == 0);

    ASSERT_INT_EQUALS("Peek of heap returns minimum value in heap.", (int)heap_peek(h), 0);

    heap_destroy(h);
})

TEST_CASE(test_heap_push_multiple_elements_assert_min_popped, {
    heap *h = heap_new(int_compare);

    for (int i = 0; i < 10; i++)
    {
        heap_node *node;
        h = heap_push(h, i, &node);
        ASSERT_TRUE("All invariants are satisfied", validate_invariants(h) == 0);
    }

    for (int i = 0; i < 10; i++)
    {
        int peeked_value = (int)heap_peek(h);
        ASSERT_INT_EQUALS("Peek of heap returns minimum value in heap.", peeked_value, i);

        int popped = (int)heap_pop(h);
        ASSERT_INT_EQUALS("Pop of heap with returns the minimum value in the heap.", popped, i);

        ASSERT_TRUE("All invariants are satisfied", validate_invariants(h) == 0);
    }

    heap_destroy(h);
})

TEST_CASE(test_heap_decrease_key_of_elements_in_order, {
    int total_elements = 10;

    heap *h = heap_new(int_compare);

    srandom(457);

    for (int i = 1; i <= total_elements; i++)
    {
        heap_node *node;
        h = heap_push(h, i, &node);
        ASSERT_TRUE("All invariants are satisfied", validate_invariants(h) == 0);

        heap_decrease_key(h, node, -i);
        ASSERT_TRUE("All invariants are satisfied", validate_invariants(h) == 0);
    }

    long int previous_value = -__INT_MAX__;

    for (int i = total_elements; i > 0; i--)
    {
        int popped = (int)heap_pop(h);
        ASSERT_INT_EQUALS("Heap sort gets the expected element", popped, -i);
        ASSERT_TRUE("All invariants are satisfied", validate_invariants(h) == 0);
        previous_value = popped;
    }

    heap_destroy(h);
})

char test_heap_sort_of_random_elements(int seed, int N)
{
    int total_elements = N;
    char assertion_error = 0;

    heap *h = heap_new(int_compare);

    srandom(seed);

    for (int i = 0; i < total_elements; i++)
    {
        int r = (int)random();
        heap_node *node;
        h = heap_push(h, r, &node);
        ASSERT_TRUE("All invariants are satisfied", validate_invariants(h) == 0);
    }

    long int previous_value = -__INT_MAX__;

    for (int i = 0; i < total_elements; i++)
    {
        int popped = (int)heap_pop(h);
        ASSERT_TRUE("Heap sort gets elements to increasing order", popped >= previous_value);
        ASSERT_TRUE("All invariants are satisfied", validate_invariants(h) == 0);
        previous_value = popped;
    }

    heap_destroy(h);

    return assertion_error;
}

TEST_CASE(test_heap_sort_0, { assertion_error = test_heap_sort_of_random_elements(0, 250); });

TEST_CASE(test_heap_sort_1, { assertion_error = test_heap_sort_of_random_elements(1, 250); });

TEST_CASE(test_heap_sort_2, { assertion_error = test_heap_sort_of_random_elements(2, 250); });

TEST_CASE(test_heap_sort_3, { assertion_error = test_heap_sort_of_random_elements(3, 250); });

TEST_CASE(test_heap_sort_4, { assertion_error = test_heap_sort_of_random_elements(4, 250); });

// TODO: For every write operation of the heap, assert the invariants.

TEST_SUITE(
        RUN_TEST("Test destroying an empty heap.", &test_destroy_empty_heap),
        RUN_TEST("Test push pop of an item in a heap works.", &test_heap_push_one_then_peek_assert_equals),
        RUN_TEST("Test push multiple items in reverse order and check min.", &test_heap_push_multiple_elements_in_reverse_order_assert_min_peek),
        RUN_TEST("Test push multiple items in random order and check min.", &test_heap_push_multiple_elements_in_random_order_assert_min_peek),
        RUN_TEST("Test push multiple items in order and pop min all of them.", &test_heap_push_multiple_elements_assert_min_popped),
        RUN_TEST("Test decrease key operations.", &test_heap_decrease_key_of_elements_in_order),
        // RUN_TEST("Test heap sorting several random elements.", &test_heap_sort_0),
        // RUN_TEST("Test heap sorting several random elements.", &test_heap_sort_1),
        // RUN_TEST("Test heap sorting several random elements.", &test_heap_sort_2),
        // RUN_TEST("Test heap sorting several random elements.", &test_heap_sort_3),
        // RUN_TEST("Test heap sorting several random elements.", &test_heap_sort_4),
)
