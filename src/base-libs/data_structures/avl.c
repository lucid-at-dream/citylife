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

    // TODO: Rebalance after remove
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
}

void tree_remove_node_with_two_subtrees(avl_tree *tree, avl_node *node)
{
    // find the leftmost node in the right subtree
    avl_node *leftmost = node->right;
    while (leftmost->left != NULL)
    {
        leftmost = leftmost->left;
    }

    // remove it
    tree_remove(tree, leftmost);

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

    leftmost->parent = node->parent;

    leftmost->right = node->right;
    if (leftmost->right != NULL)
    {
        leftmost->right->parent = leftmost;
    }

    leftmost->left = node->left;
    if (leftmost->left != NULL)
    {
        leftmost->left->parent = leftmost;
    }
}

// Private functions

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
            int relation = compare(new_node->data, this->left->data);
            if (relation > 0) // Node was inserted in the right subtree of the left node. Rotate subtree left.
            {
                rotate_left(tree, this->left);
            }
            rotate_right(tree, this);
        }
        else // Right branch is too long
        {
            int relation = compare(new_node->data, this->right->data);
            if (relation < 0)
            {
                rotate_right(tree, this->right); // Node was inserted in the left subtree of the right node. Rotate it right.
            }
            rotate_left(tree, this);
        }
    }

    // Update node height
    update_height(this);
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
