#include "../ctest/test.h"
#include "../ctest/assert.h"

#include "list.h"

#include <stdlib.h>

TEST_CASE(test_list_append_and_delete_first, {
        list *l = list_new();

        for (int i = 0; i < 10; i++) {
                list_append(l, i);
        }

        ASSERT_INT_EQUALS("First item is 0", list_get_first(l), 0);
        ASSERT_INT_EQUALS("Last item is 9", list_get_last(l), 9);

        for (int i = 0; i < 10; i++) {
                int value = (int)list_del_first(l);
                ASSERT_INT_EQUALS("The response should be the same as the data sent over.", value, i);
        }

        ASSERT_TRUE("List should be empty.", list_is_empty(l));

        list_destroy(l);

        return assertion_error;
})

TEST_CASE(test_list_prepend_and_delete_first, {
        list *l = list_new();

        for (int i = 0; i < 10; i++) {
                list_prepend(l, i);
        }

        for (int i = 9; i >= 0; i--) {
                int value = (int)list_del_first(l);
                ASSERT_INT_EQUALS("The response should be the same as the data sent over.", value, i);
        }

        ASSERT_TRUE("List should be empty.", list_is_empty(l));

        list_destroy(l);
})

TEST_CASE(test_list_delete_specific_element, {
        list *l = list_new();

        for (int i = 0; i < 10; i++) {
                list_prepend(l, i);
        }

        ASSERT_TRUE("Existing element is successfully deleted", list_del_element(l, 5));
        ASSERT_FALSE("Non existing element is not deleted", list_del_element(l, 5));

        // TODO: This is not retrieving the last digit because the last one is "0" which matches with the NULL check -.-
        int count = 0;
        int value;
        while (!list_is_empty(l)) {
                value = list_del_first(l);
                count += 1;
        }
        ASSERT_INT_EQUALS("List of 10 elements should have 9 after 1 is deleted", count, 9);

        ASSERT_TRUE("List should be empty.", list_is_empty(l));
        list_destroy(l);
})

TEST_CASE(test_list_with_negative_numbers, {
        list *l = list_new();

        for (int i = 0; i > -10; i--) {
                list_prepend(l, i);
        }

        for (int i = 0; i > -10; i--) {
                int value = list_del_last(l);
                ASSERT_INT_EQUALS("List of 10 elements should have 9 after 1 is deleted", value, i);
        }

        ASSERT_TRUE("Attempting to get a value from the empty list holds a NULL pointer", list_del_first(l) == NULL);
        ASSERT_TRUE("Attempting to get a value from the empty list holds a NULL pointer", list_del_last(l) == NULL);

        list_destroy(l);
})

TEST_CASE(destroy_non_empty_list_check_memory_leaks, {
        list *l = list_new();

        for (int i = 0; i > -10; i--) {
                list_prepend(l, i);
        }

        list_destroy(l);
})

void increment_element(int *element)
{
        (*element) += 1;
}

TEST_CASE(apply_a_function_on_each_element_of_the_list, {
        list *l = list_new();

        for (int i = 0; i < 10; i++) {
                list_append(l, i);
        }

        list_foreach(l, increment_element);

        for (int i = 1; i <= 10; i++) {
                int value = list_del_first(l);
                ASSERT_INT_EQUALS("Value must have been incremented by foreach cycle", value, i);
        }

        list_destroy(l);
})

TEST_SUITE(RUN_TEST("Test appending and checking contents by getting the first element.",
                    test_list_append_and_delete_first),
           RUN_TEST("Test prepending and checking contents by getting the first element.",
                    test_list_prepend_and_delete_first),
           RUN_TEST("Test deleting a specific item in a list", test_list_delete_specific_element),
           RUN_TEST("Test using negative numbers in the list just for fun", test_list_with_negative_numbers),
           RUN_TEST("Test that destroying a non empty list does not introduce memory leaks",
                    destroy_non_empty_list_check_memory_leaks),
           RUN_TEST("Test that a routine applied foreach element of the list can change the listed values",
                    apply_a_function_on_each_element_of_the_list))
