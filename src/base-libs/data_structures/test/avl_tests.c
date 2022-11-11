#include "assert.h"
#include "test.h"

#include "avl.h"

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

int int_compare_ptr(const void *a, const void *b)
{
    int int_a = *(int *)a;
    int int_b = *(int *)b;

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
    avl_tree *tree = tree_new(int_compare);

    tree_insert(tree, 1);

    avl_node *node_found = tree_find(tree, 1, int_compare);
    int data = (int)(node_found->data);

    ASSERT_INT_EQUALS("Node found has the expected value", data, 1);

    tree_destroy(tree);
})

TEST_CASE(test_tree_insert_some_find_some, {
    avl_tree *tree = tree_new(int_compare);

    tree_insert(tree, 1);
    tree_insert(tree, 3);
    tree_insert(tree, 2);
    tree_insert(tree, 6);
    tree_insert(tree, 5);
    tree_insert(tree, 8);
    tree_insert(tree, 4);
    tree_insert(tree, 0);
    tree_insert(tree, 9);
    tree_insert(tree, 7);

    avl_node *find_3 = tree_find(tree, 3, int_compare);
    int data_3 = (int)(find_3->data);

    avl_node *find_4 = tree_find(tree, 4, int_compare);
    int data_4 = (int)(find_4->data);

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
    avl_tree *tree = tree_new(int_compare);

    int N = 28;

    int num_finds = 10;
    int max_height = (int)(log(N + num_finds + 2) / log(1.618) - 0.3277);

    // Insert N random elements
    srand(0);
    for (int i = 0; i < N - num_finds; i++)
    {
        tree_insert(tree, rand() % 9999);
    }

    // Insert a few more
    int stuff_to_find[num_finds];
    for (int i = 0; i < num_finds; i++)
    {
        stuff_to_find[i] = rand() % 9999;
        tree_insert(tree, stuff_to_find[i]);
    }

    for (int i = 0; i < num_finds; i++)
    {
        global_comparison_counter = 0;
        tree_find(tree, stuff_to_find[i], int_compare_with_global_counter);
        ASSERT_INT_LESS_THAN("Number of comparisons should never exceed log2(N)", global_comparison_counter, max_height + 1);
    }

    tree_destroy(tree);
})

TEST_CASE(test_rebalance_after_remove, {
    avl_tree *tree = tree_new(int_compare);

    int N = 50;
    int removals = 30;
    int num_finds = 10;

    int max_height = (int)(log(N - removals + num_finds + 2) / log(1.618) - 0.3277);

    int stuff_to_insert[N + num_finds];

    // Sort a random list of ints for insertion
    srand(0);
    for (int i = 0; i < N + num_finds; i++)
    {
        stuff_to_insert[i] = rand() % 100000000;
    }

    qsort(stuff_to_insert, N + num_finds, sizeof(int), int_compare_ptr);

    // Insert them in order
    for (int i = 0; i < N + num_finds; i++)
    {
        tree_insert(tree, stuff_to_insert[i]);
    }

    // Remove a few in order
    for (int i = 0; i < removals; i++)
    {
        avl_node *node = tree_find(tree, stuff_to_insert[i], int_compare);
        ASSERT_NOT_NULL("A node that is inserted can be found", node);
        if (node != NULL)
        {
            tree_remove(tree, node);
        }
        free(node);
    }

    for (int i = N; i < N + num_finds; i++)
    {
        global_comparison_counter = 0;
        tree_find(tree, stuff_to_insert[i], int_compare_with_global_counter);
        ASSERT_INT_LESS_THAN("Number of comparisons should never exceed log2(N)", global_comparison_counter, max_height + 1);
    }

    tree_destroy(tree);
})

void callback(avl_node *data)
{
    int left = data->left != NULL ? (int)(data->left->data) : 0;
    int right = data->right != NULL ? (int)(data->right->data) : 0;
    int parent = data->parent != NULL ? (int)(data->parent->data) : 0;

    printf("Node: %d (parent: %d) (left: %d) (right: %d)\n", data->data, parent, left, right);
}

TEST_CASE(test_tree_insert_remove_random_data, {
    avl_tree *tree = tree_new(int_compare);

    int N = 100;
    int stuff_to_insert[N];

    // Sort a random list of ints for insertion
    srand(0);
    for (int i = 0; i < N; i++)
    {
        stuff_to_insert[i] = rand() % 100000000;
    }

    // Insert and remove randomly
    int removals = 0;
    for (int i = 0; i < N; i++)
    {
        tree_insert(tree, stuff_to_insert[i]);

        if (rand() % 5 == 0)
        {
            avl_node *n = tree_find(tree, stuff_to_insert[removals], int_compare);
            tree_remove(tree, n);
            removals++;
            free(n);
        }
    }

    // int max_height = (int)(log(N - removals + 2) / log(1.618) - 0.3277);
    int max_height = ceil(log(N - removals) / log(2)) + 1;
    for (int i = removals; i < N; i++)
    {
        global_comparison_counter = 0;
        tree_find(tree, stuff_to_insert[i], int_compare_with_global_counter);
        ASSERT_INT_LESS_THAN("Number of comparisons should never exceed log2(N)", global_comparison_counter, max_height + 1);
    }

    tree_destroy(tree);
})

TEST_SUITE(
    RUN_TEST("Test inserting and retrieving an element.", &test_tree_insert_one_find_one),
    RUN_TEST("Test inserting and retrieving some elements.", &test_tree_insert_some_find_some),
    RUN_TEST("Test comparison count of find operations after inserts.", &test_tree_insert_random_assert_logarithmic_comparison_count),
    RUN_TEST("Test logarithmic comparison after in-order inserts followed by removes.", &test_rebalance_after_remove),
    RUN_TEST("Test random insertions and remotions", &test_tree_insert_remove_random_data))
