#include "../ctest/test.h"
#include "../ctest/assert.h"

#include "list.h"

TEST_CASE(test_list_append_and_delete_first,
    {
        list *l = list_new();
        
        for (int i = 0; i < 10; i++) {
            list_append(l, i);
        }

        for (int i = 0; i < 10; i++) {
            int value = (int)list_del_first(l);
            ASSERT_INT_EQUALS("The response should be the same as the data sent over.", value, i);
        }

        ASSERT_TRUE("List should be empty.", list_is_empty(l));

        list_destroy(l);

        return assertion_error;
    }
)

TEST_CASE(test_list_prepend_and_delete_first,
    {
        list *l = list_new();
    
        for (int i = 0; i < 10; i++) {
            list_prepend(l, i);
        }

        for (int i = 0; i < 10; i++) {
            int value = (int)list_del_first(l);
            ASSERT_INT_EQUALS("The response should be the same as the data sent over.", value, 9-i);
        }

        ASSERT_TRUE("List should be empty.", list_is_empty(l));

        list_destroy(l);
    }
)

TEST_SUITE (
    RUN_TEST("Test appending and checking contents by getting the first element.", test_list_append_and_delete_first),
    RUN_TEST("Test prepending and checking contents by getting the first element.", test_list_prepend_and_delete_first)
)
