#pragma once

typedef struct _avl_node
{
    void *data;
    int height;
    struct _avl_node *parent;
    struct _avl_node *left;
    struct _avl_node *right;
} avl_node;

typedef struct _avl_tree
{
    unsigned int size;
    avl_node *root;
} avl_tree;

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
avl_node *find(avl_tree *tree, void *data, int (*compare)(const void *given_data, const void *node_data));

/**
 * @brief Removes the given node from the given tree.
 * 
 * @param tree The tree from which the node will be removed.
 * @param node The node that will be removed.
 */
void tree_remove(avl_tree *tree, avl_node *node);

/**
 * @brief Merges the contents of the smallest tree given into the other tree. After this operation the
 * smallest tree will be empty and deallocated.
 * 
 * @param a_tree A tree to be subject to the merge operation.
 * @param another_tree Another tree to be subject to the merge operation.
 * @param merge A function that will be called when the two items are equal. It should return the desired
 * merge resolution.
 * @param compare A function capable of comparing the data of a_tree with the data of another_tree. 
 * Returns -1, 0, 1 in case the given item from a_tree is smaller, equal or larger than the item from 
 * another_tree that is being compared, respectively.
 * @return avl_tree* A pointer to the tree that received the elements of the other tree.
 */
avl_tree *tree_merge(avl_tree *a_tree, avl_tree *another_tree, char overwrite, void *(*merge)(const void *a_tree_data, const void *another_tree_data),
                     int (*compare)(const void *a_tree_data, const void *another_tree_data));
