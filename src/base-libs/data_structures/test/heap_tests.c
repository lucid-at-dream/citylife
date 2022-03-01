#include "test.h"
#include "assert.h"

#include "heap.h"

#include <stdlib.h>
#include <math.h>

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

char is_active(heap_node *n) {
    return n != NULL && n->activity != NULL && n->activity->is_active;
}

char check_node_structure(heap_node *node) {
    char assertion_error = 0;

    if (node->children != NULL && node->children->head != NULL) {
        list_node *c_ln = node->children->head;

        while (c_ln != NULL) {
            heap_node *c = c_ln->value;
            if (is_active(c)) {
                ASSERT_TRUE("No passive nodes to the left of active nodes", c_ln->prev == NULL || is_active(c_ln->prev->value));
            }

            assertion_error += check_node_structure(c);
            c_ln = c_ln->next;
        }
    }
    return assertion_error;
}

char check_active_nodes_active_child_rank_plus_loss(heap_node *node) {
    char assertion_error = 0;

    if (node->children != NULL && node->children->head != NULL) {
        list_node *c_ln = node->children->head;
        list_node *rightmost_active_child = NULL;
        int rightmost_found = 0;

        while (c_ln != NULL) {
            heap_node *c = c_ln->value;
            if (!is_active(c) && !rightmost_found) {
                rightmost_active_child = c_ln->prev;
                rightmost_found = 1;
            }

            // Call recursively for all children.
            check_active_nodes_active_child_rank_plus_loss(c);

            c_ln = c_ln->next;
        }
        
        // Assert rank+loss of active children for active nodes.
        if (is_active(node)) {
            int i = 0;
            while (rightmost_active_child != NULL) {
                heap_node *c = rightmost_active_child->value;
                rank_list_record *rank = c->rank->value;
                ASSERT_TRUE("i-th rightmost active child has rank+loss at least i-1", rank->value + c->loss <= i);
                rightmost_active_child = rightmost_active_child->prev;
            }
        }
    }
    return assertion_error;
}

char is_linkable(heap_node *x) {
    if (x == NULL) {
        return 0;
    }

    if (!is_active(x)) {
        if (x->children == NULL) {
            return 1;
        }

        list_node *child = x->children->head;
        while (child != NULL) {
            if (is_active(child->value)) {
                return 0;
            }
            child = child->next;
        }
        return 1;
    }
    return 0;
}

char check_structure(heap* h) {

    if (h->root == NULL) {
        return 0;
    }

    char assertion_error = 0;

    ASSERT_FALSE("Root node is passive", is_active(h->root));

    if (h->root->children != NULL && h->root->children->tail != NULL) {
        list_node *c_ln = h->root->children->tail;

        heap_node *c = c_ln->value;
        while (c != NULL && is_linkable(c)) {
            c_ln = c_ln->prev;
            c = c_ln != NULL ? c_ln->value : NULL;
        }

        while(c_ln != NULL) {
            c = c_ln->value;
            ASSERT_FALSE("Linkable nodes are the rightmost children of the root", is_linkable(c));
            c_ln = c_ln->prev;
        }
    }

    assertion_error += check_node_structure(h->root);
    assertion_error += check_active_nodes_active_child_rank_plus_loss(h->root);

    return assertion_error;
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

char is_active_root(heap_node *x) {
    return is_active(x) && !is_active(x->parent); // heap->root is always passive. Should never NULL derreference.
}

int count_active_roots(heap_node *n) {
    if (n == NULL) {
        return 0;
    }

    int count = 0;
    if (is_active_root(n)) {
        count += 1;
    }

    if (n->children != NULL && n->children->head != NULL) {
        list_node *c_ln = n->children->head;
        while (c_ln != NULL) {
            heap_node *c = c_ln->value;
            count += count_active_roots(c);
            c_ln = c_ln->next;
        }
    }

    return count;
}

char check_total_number_of_active_roots(heap *h) {
    if (h == NULL || h->root == NULL) {
        return 0;
    }
    
    char assertion_error = 0;

    double R = 2 * log2(h->size) + 6;

    int active_root_count = count_active_roots(h->root);

    ASSERT_INT_LESS_THAN("The total number of active roots is at most R + 1", active_root_count, R + 2);
    
    return assertion_error;
}

int count_total_loss(heap_node *n) {
    if (n == NULL) {
        return 0;
    }

    int count = n->loss;

    if (n->children != NULL && n->children->head != NULL) {
        list_node *c_ln = n->children->head;
        while (c_ln != NULL) {
            heap_node *c = c_ln->value;
            count += count_total_loss(c);
            c_ln = c_ln->next;
        }
    }

    return count;
}

char check_total_loss(heap *h) {
    if (h == NULL || h->root == NULL) {
        return 0;
    }
    
    char assertion_error = 0;

    double R = 2 * log2(h->size) + 6;

    int total_loss = count_total_loss(h->root);

    ASSERT_TRUE("The total loss is at most R + 1", total_loss <= R + 1);
    
    return assertion_error;
}

char check_root_degree(heap *h) {
    if (h == NULL || h->root == NULL) {
        return 0;
    }

    char assertion_error = 0;
    double R = 2 * log2(h->size) + 6;

    if (h->root->children != NULL) {
        ASSERT_INT_LESS_THAN("The maximum degree of the root is R + 3", h->root->children->size, R + 4);
    }

    return assertion_error;
}

char validate_invariants(heap *h) {
    char assertion_error = 0;
    assertion_error += check_heap_order(h->root, h->compare);
    assertion_error += assert_all_elemenets_in_Q(h);
    assertion_error += check_structure(h);
    assertion_error += check_total_number_of_active_roots(h);
    assertion_error += check_root_degree(h);
    // TODO: Check degree of non-root nodes
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

char test_heap_sort_of_random_elements(int seed, int N) {
    int total_elements = N;
    char assertion_error = 0;

    heap *h = heap_new(int_compare);

    srandom(seed);

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

    return assertion_error;
}

TEST_CASE(test_heap_sort_0, {
    assertion_error = test_heap_sort_of_random_elements(0, 250);
});

TEST_CASE(test_heap_sort_1, {
    assertion_error = test_heap_sort_of_random_elements(1, 250);
});

TEST_CASE(test_heap_sort_2, {
    assertion_error = test_heap_sort_of_random_elements(2, 250);
});

TEST_CASE(test_heap_sort_3, {
    assertion_error = test_heap_sort_of_random_elements(3, 250);
});

TEST_CASE(test_heap_sort_4, {
    assertion_error = test_heap_sort_of_random_elements(4, 250);
});

// TODO: For every write operation of the heap, assert the invariants.

TEST_SUITE(
    RUN_TEST("Test destroying an empty heap.", &test_destroy_empty_heap),
    RUN_TEST("Test push pop of an item in a heap works.", &test_heap_push_one_then_peek_assert_equals),
    RUN_TEST("Test push multiple items in reverse order and check min.", &test_heap_push_multiple_elements_in_reverse_order_assert_min_peek),
    RUN_TEST("Test push multiple items in random order and check min.", &test_heap_push_multiple_elements_in_random_order_assert_min_peek),
    RUN_TEST("Test push multiple items in order and pop min all of them.", &test_heap_push_multiple_elements_assert_min_popped),
    // RUN_TEST("Test heap sorting several random elements.", &test_heap_sort_0),
    // RUN_TEST("Test heap sorting several random elements.", &test_heap_sort_1),
    // RUN_TEST("Test heap sorting several random elements.", &test_heap_sort_2),
    // RUN_TEST("Test heap sorting several random elements.", &test_heap_sort_3),
    // RUN_TEST("Test heap sorting several random elements.", &test_heap_sort_4),
)
