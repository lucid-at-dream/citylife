#pragma once

typedef struct _avl_node
{
    void *data;
    char balance_factor;
    struct _avl_node *parent;
    struct _avl_node *left;
    struct _avl_node *right;
} avl_node;

typedef struct _avl_tree
{
    unsigned int size;
    avl_node *root;
    int (*compare)(const void *, const void *);
} avl_tree;

void pretty_print(avl_tree *tree);

/**
 * @brief Creates a new tree that will use the given compare function for sorting.
 * 
 * @param compare A function to compare data in nodes, should return -1, 0, 1 in case the first
 * argument is less than, equal to or greater than the second argument, respectively.
 * @return avl_tree* The newly allocated tree.
 */
avl_tree *tree_new(int (*compare)(const void *, const void *));

/**
 * @brief Deallocates all nodes of a tree and the tree itself. The data is not deallocated.
 * 
 * @param tree 
 */
void tree_destroy(avl_tree *tree);

/**
 * @brief Inserts the given data in the given tree
 * 
 * @param tree The tree in which the data will be inserted
 * @param data The data that will be inserted
 */
void tree_insert(avl_tree *tree, void *data);

/**
 * @brief Retrieves a node for which the given function compare returns 0.
 * 
 * @param tree The tree in which to search
 * @param data The data that will be searched for
 * @param compare A comparison function that will be called with the given data as first argument 
 * and a node's data as second argument. The function should return -1, 0 or 1 if the given data
 * is less, equal or greater than the node's data, respectively.
 * @return avl_node* The tree node that contains the found data.
 */
avl_node *tree_find(avl_tree *tree, void *data, int (*compare)(const void *given_data, const void *node_data));

/**
 * @brief Removes the given node from the given tree.
 * 
 * @param tree The tree from which the node will be removed.
 * @param node The node that will be removed.
 */
void tree_remove(avl_tree *tree, avl_node *node);

void tree_pre_order(avl_tree *tree, void (*callback)(const avl_node *data));