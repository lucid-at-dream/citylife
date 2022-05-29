#include "avl.h"

#include <stdlib.h>

avl_node *avl_node_new(void *data);
void avl_node_destroy(avl_node *node);

void tree_node_destroy_recurse(avl_node *node);
void tree_node_insert_recurse(avl_tree *tree, avl_node *node, avl_node *new_node, int (*compare)(const void *, const void *));
avl_node *tree_node_find(avl_node *this, void *data, int (*compare)(const void *, const void *));
void rotate_left(avl_tree *tree, avl_node *this);
void rotate_right(avl_tree *tree, avl_node *this);
void update_height(avl_node *this);
void tree_remove_leaf_node(avl_tree *tree, avl_node *node);
void tree_remove_node_with_one_subtree(avl_tree *tree, avl_node *node);
void tree_remove_node_with_two_subtrees(avl_tree *tree, avl_node *node);
void tree_rebalance_node(avl_tree *tree, avl_node *unbalanced_node);
void tree_pre_order_recurse(avl_node *node, void (*callback)(const avl_node *data));

avl_tree *tree_new(int (*compare)(const void *, const void *))
{
    avl_tree *new = (avl_tree *)calloc(1, sizeof(avl_tree));
    new->compare = compare;
    return new;
}

void tree_destroy(avl_tree *tree)
{
    tree_node_destroy_recurse(tree->root);
    free(tree);
}

void tree_insert(avl_tree *tree, void *data)
{
    avl_node *new_node = avl_node_new(data);

    if (tree->root == NULL)
    {
        tree->root = new_node;
    }
    else
    {
        tree_node_insert_recurse(tree, tree->root, new_node, tree->compare);
    }
}

avl_node *tree_find(avl_tree *this, void *data, int (*compare)(const void *given_data, const void *node_data))
{
    avl_node *node = this->root;

    while (node != NULL)
    {
        int relation = compare(data, node->data);
        if (relation > 0)
        {
            node = node->right;
        }
        else if (relation < 0)
        {
            node = node->left;
        }
        else
        {
            return node;
        }
    }
    return NULL;
}

void tree_remove(avl_tree *tree, avl_node *node)
{
    if (node == NULL)
    {
        return;
    }

    // if there is no offspring, we can delete right away
    if (node->left == NULL && node->right == NULL)
    {
        tree_remove_leaf_node(tree, node);
    }

    // if there's only one child, we can just replace node node for it
    else if (node->left == NULL || node->right == NULL)
    {
        tree_remove_node_with_one_subtree(tree, node);
    }

    // if we have two children, we need to fetch the leftmost node of the right subtree and replace this node for it.
    else
    {
        tree_remove_node_with_two_subtrees(tree, node);
    }

    // Starting at the first node that may be unbalanced
    avl_node *unbalanced_node = node->parent;
    if (unbalanced_node == NULL)
    {
        unbalanced_node = tree->root;
    }

    // Let's go up checking the balance factor and applying rotations accordingly
    while (unbalanced_node != NULL)
    {
        tree_rebalance_node(tree, unbalanced_node);
        unbalanced_node = unbalanced_node->parent;
    }
}

void tree_pre_order(avl_tree *tree, void (*callback)(const avl_node *data))
{
    tree_pre_order_recurse(tree->root, callback);
}

// Private functions

void tree_pre_order_recurse(avl_node *node, void (*callback)(const avl_node *data))
{
    if (node == NULL)
    {
        return;
    }

    tree_pre_order_recurse(node->left, callback);
    callback(node);
    tree_pre_order_recurse(node->right, callback);
}

void tree_remove_leaf_node(avl_tree *tree, avl_node *node)
{
    if (node->parent != NULL)
    {
        if (node == node->parent->left)
        {
            node->parent->left = NULL;
        }
        else
        {
            node->parent->right = NULL;
        }
    }
    else
    {
        tree->root = NULL;
    }
}

void tree_remove_node_with_one_subtree(avl_tree *tree, avl_node *node)
{
    avl_node *subtree = node->left == NULL ? node->right : node->left;

    if (node->parent != NULL)
    {
        if (node->parent->right == node)
        {
            node->parent->right = subtree;
        }
        else
        {
            node->parent->left = subtree;
        }
    }
    else
    {
        tree->root = subtree;
    }
    subtree->parent = node->parent;
    subtree->height = node->height;
}

void tree_remove_node_with_two_subtrees(avl_tree *tree, avl_node *node)
{
    // find the leftmost node in the right subtree
    avl_node *leftmost = node->right;
    while (leftmost->left != NULL)
    {
        leftmost = leftmost->left;
    }

    // Remove that node.
    // A possible right subtree of this leftmost node becomes the son of the leftmost node parent
    if (leftmost != node->right)
    {
        if (leftmost->right != NULL)
        {
            leftmost->parent->left = leftmost->right;
            leftmost->right->parent = leftmost->parent;
        }
        else
        {
            leftmost->parent->left = NULL;
        }
        update_height(leftmost->parent);
    }
    else
    {
    }

    // replace the node being removed by the leftmost node of the right subtree
    if (node->parent != NULL)
    {
        if (node->parent->right == node)
        {
            node->parent->right = leftmost;
        }
        else
        {
            node->parent->left = leftmost;
        }
    }
    else
    {
        tree->root = leftmost;
    }

    // Make leftmost node aware that it is occupying the old node's position
    leftmost->parent = node->parent;
    leftmost->height = node->height;
    if (leftmost != node->right)
    {
        leftmost->right = node->right;
        node->right->parent = leftmost;
    }
    leftmost->left = node->left;
    node->left->parent = leftmost;
}

avl_node *avl_node_new(void *data)
{
    avl_node *new = (avl_node *)calloc(1, sizeof(avl_node));
    new->data = data;
    return new;
}

void avl_node_destroy(avl_node *node)
{
    free(node);
}

void tree_node_destroy_recurse(avl_node *node)
{
    if (node == NULL)
    {
        return;
    }

    tree_node_destroy_recurse(node->left);
    tree_node_destroy_recurse(node->right);

    avl_node_destroy(node);
}

void tree_node_insert_recurse(avl_tree *tree, avl_node *this, avl_node *new_node, int (*compare)(const void *, const void *))
{
    int compar = compare(new_node->data, this->data);

    // Insert the node
    if (compar > 0)
    {
        if (this->right == NULL)
        {
            this->right = new_node;
            new_node->parent = this;
        }
        else
        {
            tree_node_insert_recurse(tree, this->right, new_node, compare);
        }
    }
    else if (compar < 0)
    {
        if (this->left == NULL)
        {
            this->left = new_node;
            new_node->parent = this;
        }
        else
        {
            tree_node_insert_recurse(tree, this->left, new_node, compare);
        }
    }

    // Update node height
    update_height(this);

    // Rebalance if need be
    tree_rebalance_node(tree, this);
}

void tree_rebalance_node(avl_tree *tree, avl_node *this)
{
    // Check if this subtree is unbalanced
    int hleft = this->left != NULL ? this->left->height : 0;
    int hright = this->right != NULL ? this->right->height : 0;
    int balance = hleft - hright;

    // rebalance
    if (abs(balance) > 1)
    {
        // Left branch is too long
        if (hleft > hright)
        {
            int lr_height = this->left->right != NULL ? this->left->right->height : 0, ll_height = this->left->left != NULL ? this->left->left->height : 0;

            if (lr_height > ll_height)
            {
                rotate_left(tree, this->left); // Left-Right
            }
            rotate_right(tree, this);
        }
        else // Right branch is too long
        {
            int rl_height = this->right->left != NULL ? this->right->left->height : 0, rr_height = this->right->right != NULL ? this->right->right->height : 0;

            if (rl_height > rr_height)
            {
                rotate_right(tree, this->right); // Right-Left
            }
            rotate_left(tree, this);
        }
    }
}

void rotate_left(avl_tree *tree, avl_node *this)
{
    if (this->right == NULL)
    {
        return;
    }

    avl_node *new_parent = this->right;
    avl_node *grandfather = this->parent;

    // My right son's left son becomes my right son.
    this->right = new_parent->left;
    if (this->right != NULL)
    {
        this->right->parent = this;
    }

    // I become my right child's left son.
    new_parent->left = this;
    this->parent = new_parent;
    new_parent->parent = grandfather;

    if (grandfather != NULL)
    {
        if (grandfather->left == this)
        {
            grandfather->left = new_parent;
        }
        else
        {
            grandfather->right = new_parent;
        }
    }
    else
    {
        tree->root = new_parent;
    }

    // Update node height
    update_height(this);
    update_height(new_parent);
}

void rotate_right(avl_tree *tree, avl_node *this)
{
    if (this->left == NULL)
    {
        return;
    }

    avl_node *new_parent = this->left;
    avl_node *grandfather = this->parent;

    // My left son's right son becomes my left son.
    this->left = new_parent->right;
    if (this->left != NULL)
    {
        this->left->parent = this;
    }

    // I become my right child's left son.
    new_parent->right = this;
    this->parent = new_parent;
    new_parent->parent = grandfather;

    if (grandfather != NULL)
    {
        if (grandfather->left == this)
        {
            grandfather->left = new_parent;
        }
        else
        {
            grandfather->right = new_parent;
        }
    }
    else
    {
        tree->root = new_parent;
    }

    // Update node height
    update_height(this);
    update_height(new_parent);
}

void update_height(avl_node *this)
{
    int hleft = this->left != NULL ? this->left->height : 0;
    int hright = this->right != NULL ? this->right->height : 0;

    this->height = hleft > hright ? hleft + 1 : hright + 1;
}
