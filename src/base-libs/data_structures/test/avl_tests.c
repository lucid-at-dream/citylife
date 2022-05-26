#include "assert.h"
#include "test.h"

#include "avl.h"

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

TEST_CASE(test_tree_insert_one_find_one, {
    avl_tree *tree = tree_new(&int_compare);

    tree_insert(tree, 1);

    avl_node *node_found = tree_find(tree, 1, int_compare);
    int data = (int)(node_found->data);

    ASSERT_INT_EQUALS("Node found has the expected value", data, 1);

    tree_destroy(tree);
})

TEST_CASE(test_tree_insert_some_find_some, {
    avl_tree *tree = tree_new(&int_compare);

    tree_insert(tree, 1);
    tree_insert(tree, 3);
    tree_insert(tree, 5);
    tree_insert(tree, 4);
    tree_insert(tree, 2);

    avl_node *find_3 = tree_find(tree, 3, int_compare);

    if (find_3 == NULL)
    {
        printf("NULL\n");
    }

    int data_3 = (int)(find_3->data);
    printf("yay: %d\n", data_3);

    avl_node *find_4 = tree_find(tree, 4, int_compare);
    int data_4 = (int)(find_4->data);
    printf("yay: %d\n", data_4);

    ASSERT_INT_EQUALS("Node found has the expected value", data_3, 3);
    ASSERT_INT_EQUALS("Node found has the expected value", data_4, 4);

    tree_destroy(tree);
})

int global_comparison_counter = 0;

int int_compare_with_global_counter(const void *a, const void *b)
{
    global_comparison_counter += 1;
    int_compare(a, b);
}

TEST_CASE(test_tree_insert_random_assert_logarithmic_comparison_count, {
    avl_tree *tree = tree_new(&int_compare);

    int log_b2_512 = 9;
    int num_finds = 10;

    // Insert 512 random elements
    srand(0);
    for (int i = 0; i < 511 - num_finds; i++)
    {
        tree_insert(tree, rand());
    }

    // Insert a few more
    int stuff_to_find[num_finds];
    for (int i = 0; i < num_finds; i++)
    {
        stuff_to_find[i] = rand();
        tree_insert(tree, stuff_to_find[i]);
    }

    for (int i = 0; i < num_finds; i++)
    {
        global_comparison_counter = 0;
        tree_find(tree, stuff_to_find[i], int_compare_with_global_counter);
        ASSERT_INT_LESS_THAN("Number of comparisons should never exceed log2(N)", global_comparison_counter, log_b2_512 + 1);
    }

    if (1)
        printf("hey!");

    tree_destroy(tree);
})

TEST_SUITE(RUN_TEST("Test inserting and retrieving an element.", &test_tree_insert_one_find_one),
           RUN_TEST("Test inserting and retrieving some elements.", &test_tree_insert_some_find_some),
           RUN_TEST("Test comparison count of find operations", &test_tree_insert_random_assert_logarithmic_comparison_count), )
