#include "test.h"
#include "assert.h"

#include "heap.h"

#include <stdlib.h>

TEST_CASE(test_heap_push_one_pop_one_assert_equals, {
    heap *h = heap_new();

    heap_push(h, 10);
    
    ASSERT_INT_EQUALS("Peek of heap with 1 item is the item.", (int)heap_peek(h), 10);

    int popped = (int)heap_pop(h);

    ASSERT_INT_EQUALS("Pop of heap with 1 item is the item.", popped, 10);

    heap_destroy(h);
})

/*TEST_CASE(test_heap_push_multiple_elements_assert_min_popped, {
    heap *h = heap_new();

    for (int i = 0; i < 10; i++) {
        heap_push(h, i);
    }

    for (int i = 0; i < 10; i++) {
        ASSERT_INT_EQUALS("Peek of heap returns minimum value in heap.", (int)heap_peek(h), i);
        
        int popped = (int)heap_pop(h);
        ASSERT_INT_EQUALS("Pop of heap with 1 item is the item.", popped, i);
    }

    heap_destroy(h);
})*/

TEST_SUITE(
    RUN_TEST("Test push pop of an item in a heap works.", test_heap_push_one_pop_one_assert_equals),
    //RUN_TEST("Test push multiple items come in order on pop.", test_heap_push_multiple_elements_assert_min_popped)
)
