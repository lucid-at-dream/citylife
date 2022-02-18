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

char check_queue_contains_node(queue *q, heap_node *n) {
    queue_item *i = q->head;
    while (i != NULL) {
        if (i->content == n) {
            return 1;
        }
        i = i->next;
    }
    return 0;
}

char check_all_heap_nodes_in_queue_recurse(queue *q, heap_node *node) {

    int assertion_error = 0;

    ASSERT_TRUE("All heap nodes are contained in Q", check_queue_contains_node(q, node));

    if (node->children == NULL || node->children->head == NULL) {
        return assertion_error;
    }

    // Recurse to children
    list_node *c_ln = node->children->head;
    while(c_ln != NULL) {
        assertion_error += check_all_heap_nodes_in_queue_recurse(q, c_ln->value);
        c_ln = c_ln->next;
    }

    return assertion_error;
}

char assert_all_elemenets_in_Q(heap *h) {
    char assertion_error = 0;

    if (h->root == NULL || h->root->children == NULL || h->root->children->head == NULL) {
        ASSERT_INT_EQUALS("Empty heap or heap with only root node has an empty queue.", h->Q->size, 0);
        return assertion_error;
    }

    list_node *c_ln = h->root->children->head;
    while(c_ln != NULL) {
        assertion_error += check_all_heap_nodes_in_queue_recurse(h->Q, c_ln->value);
        c_ln = c_ln->next;
    }

    ASSERT_INT_EQUALS("The number of nodes in Q is the number of nodes in the heap except the root", h->Q->size, h->size - 1);

    return assertion_error;
}

char check_heap_order(heap_node *n, int (*cmp)(const void *, const void *)) {
    char assertion_error = 0;
    if (n == NULL || n->children == NULL || n->children->size == 0) {
        return assertion_error;
    }

    list_node *c_ln = n->children->head;
    while (c_ln != NULL) {
        heap_node *c = c_ln->value;
        int result = cmp(n->item, c->item);
        assertion_error += ASSERT_TRUE("Heap order is satisfied", result <= 0);
        assertion_error += check_heap_order(c, cmp);
        c_ln = c_ln->next;
    }

    return assertion_error;
}

char validate_invariants(heap *h) {
    char assertion_error = 0;
    assertion_error += check_heap_order(h->root, h->compare);
    assertion_error += assert_all_elemenets_in_Q(h);
    return assertion_error;
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
        ASSERT_TRUE("All invariants are satisfied", validate_invariants(h) == 0);
    }

    ASSERT_INT_EQUALS("Peek of heap returns minimum value in heap.", (int)heap_peek(h), 0);

    heap_destroy(h);
})

TEST_CASE(test_heap_push_multiple_elements_in_random_order_assert_min_peek, {
    heap *h = heap_new(int_compare);

    h = heap_push(h, 1);
    ASSERT_TRUE("All invariants are satisfied", validate_invariants(h) == 0);
    h = heap_push(h, 4);
    ASSERT_TRUE("All invariants are satisfied", validate_invariants(h) == 0);
    h = heap_push(h, 3);
    ASSERT_TRUE("All invariants are satisfied", validate_invariants(h) == 0);
    h = heap_push(h, 0);
    ASSERT_TRUE("All invariants are satisfied", validate_invariants(h) == 0);
    h = heap_push(h, 2);
    ASSERT_TRUE("All invariants are satisfied", validate_invariants(h) == 0);

    ASSERT_INT_EQUALS("Peek of heap returns minimum value in heap.", (int)heap_peek(h), 0);

    heap_destroy(h);
})

TEST_CASE(test_heap_push_multiple_elements_assert_min_popped, {
    heap *h = heap_new(int_compare);

    for (int i = 0; i < 10; i++) {
        h = heap_push(h, i);
        ASSERT_TRUE("All invariants are satisfied", validate_invariants(h) == 0);
    }

    for (int i = 0; i < 10; i++) {
        int peeked_value = (int)heap_peek(h);
        ASSERT_INT_EQUALS("Peek of heap returns minimum value in heap.", peeked_value, i);

        int popped = (int)heap_pop(h);
        ASSERT_INT_EQUALS("Pop of heap with returns the minimum value in the heap.", popped, i);

        ASSERT_TRUE("All invariants are satisfied", validate_invariants(h) == 0);
    }

    heap_destroy(h);
})

TEST_CASE(test_heap_sort_of_random_elements, {
    int total_elements = 200;

    heap *h = heap_new(int_compare);

    srandom(0);

    for (int i = 0; i < total_elements; i++) {
        int r = (int)random();
        h = heap_push(h, r);
        ASSERT_TRUE("All invariants are satisfied", validate_invariants(h) == 0);
    }

    long int previous_value = -__INT_MAX__;

    for (int i = 0; i < total_elements; i++) {
        int popped = (int)heap_pop(h);
        ASSERT_TRUE("Heap sort gets elements to increasing order", popped >= previous_value);
        ASSERT_TRUE("All invariants are satisfied", validate_invariants(h) == 0);
        previous_value = popped;
    }

    heap_destroy(h);
})

// TODO: For every write operation of the heap, assert the invariants.

TEST_SUITE(RUN_TEST("Test destroying an empty heap.", &test_destroy_empty_heap),
           RUN_TEST("Test push pop of an item in a heap works.", &test_heap_push_one_then_peek_assert_equals),
           RUN_TEST("Test push multiple items in reverse order and check min.", &test_heap_push_multiple_elements_in_reverse_order_assert_min_peek),
           RUN_TEST("Test push multiple items in random order and check min.", &test_heap_push_multiple_elements_in_random_order_assert_min_peek),
           RUN_TEST("Test push multiple items in order and pop min all of them.", &test_heap_push_multiple_elements_assert_min_popped),
           RUN_TEST("Test heap sorting several random elements.", &test_heap_sort_of_random_elements), )
