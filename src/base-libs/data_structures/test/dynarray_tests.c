#include "assert.h"
#include "test.h"

#include "dynarray.h"

#include <stdlib.h>

TEST_CASE(test_dynarray_add_get_items, {
    dynarray *ar = dynarray_new();

    for (int i = 0; i < 10; i++)
    {
        dynarray_add(ar, i);
        ASSERT_INT_EQUALS("Dynamic array size is correctly incremented after an insertion", i + 1, ar->size);
    }

    for (int i = 9; i >= 0; i--)
    {
        ASSERT_INT_EQUALS("Elements are correctly added to the dynamic array", i, dynarray_get(ar, i));
    }

    dynarray_destroy(ar);
})

TEST_CASE(test_dynarray_add_del_get_items, {
    dynarray *ar = dynarray_new();

    for (int i = 0; i < 10; i++)
    {
        dynarray_add(ar, i);
    }
    ASSERT_INT_EQUALS("Dynamic array size is correct after a few consecutive insertions", 10, ar->size);

    dynarray_del(ar, 4); // 0, 1, 2, 3, 5, 6, 7, 8, 9
    ASSERT_INT_EQUALS("Dynamic array size is decremented by one upon successful deletion", 9, ar->size);

    dynarray_del(ar, 7); // 0, 1, 2, 3, 5, 6, 7, 9
    ASSERT_INT_EQUALS("Dynamic array size is decremented by one upon successful deletion", 8, ar->size);

    ASSERT_INT_EQUALS("Element at index 4 is correctly deleted from the dynamic array", 3, dynarray_get(ar, 3));
    ASSERT_INT_EQUALS("Element at index 4 is correctly deleted from the dynamic array", 5, dynarray_get(ar, 4));

    ASSERT_INT_EQUALS("Element at index 7 is correctly deleted from the dynamic array", 7, dynarray_get(ar, 6));
    ASSERT_INT_EQUALS("Element at index 7 is correctly deleted from the dynamic array", 9, dynarray_get(ar, 7));

    dynarray_destroy(ar);
})

TEST_CASE(test_dynarray_add_set_del_get_items, {
    dynarray *ar = dynarray_new();

    for (int i = 0; i < 10; i++)
    {
        dynarray_add(ar, i);
    }

    dynarray_set(ar, 4, 100);
    ASSERT_INT_EQUALS("Dynamic array set sets the value correctly", 100, dynarray_get(ar, 4));

    dynarray_del(ar, 4); // 0, 1, 2, 3, 5, 6, 7, 9
    ASSERT_INT_EQUALS("Dynamic array delete a set value deletes the value correctly", 5, dynarray_get(ar, 4));
    ASSERT_INT_EQUALS("Dynamic array delete a set value deletes the value correctly", 6, dynarray_get(ar, 5));

    dynarray_destroy(ar);
})

TEST_SUITE(RUN_TEST("Test adding elements to a dynamic array and then getting them.", &test_dynarray_add_get_items),
           RUN_TEST("Test deleting a few elements from a dynamic array.", &test_dynarray_add_del_get_items),
           RUN_TEST("Test setting the value of some index", &test_dynarray_add_set_del_get_items), )
