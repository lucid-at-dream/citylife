#include "avl.h"

#include <stdlib.h>

#include <math.h>
#include <stdio.h>

avl_node *avl_node_new(void *data);
void avl_node_destroy(avl_node *node);

void tree_node_destroy_recurse(avl_node *node);
void tree_node_insert_recurse(avl_tree *tree, avl_node *node, avl_node *new_node, int (*compare)(const void *, const void *));
avl_node *tree_node_find(avl_node *this, void *data, int (*compare)(const void *, const void *));
void rotate_left(avl_tree *tree, avl_node *this);
void rotate_right(avl_tree *tree, avl_node *this);
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
            node->parent->balance_factor++;
        }
        else
        {
            node->parent->right = NULL;
            node->parent->balance_factor--;
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
            node->parent->balance_factor--;
        }
        else
        {
            node->parent->left = subtree;
            node->parent->balance_factor++;
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
    // TODO: Update balance factors in this scenario!

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
            this->balance_factor++;
        }
        else
        {
            int prev_bf = this->right->balance_factor;
            tree_node_insert_recurse(tree, this->right, new_node, compare);
            if (prev_bf == 0 && this->right->balance_factor != 0)
            {
                this->balance_factor++;
            }
        }
    }
    else if (compar < 0)
    {
        if (this->left == NULL)
        {
            this->left = new_node;
            new_node->parent = this;
            this->balance_factor--;
        }
        else
        {
            int prev_bf = this->left->balance_factor;
            tree_node_insert_recurse(tree, this->left, new_node, compare);
            if (prev_bf == 0 && this->left->balance_factor != 0)
            {
                this->balance_factor--;
            }
        }
    }

    // Rebalance if need be
    tree_rebalance_node(tree, this);

    return this->balance_factor;
}

void tree_rebalance_node(avl_tree *tree, avl_node *this)
{
    // rebalance if the balance factor is too high
    if (abs(this->balance_factor) > 1)
    {
        // Left branch is too long
        if (this->balance_factor < 0)
        {
            // right side of left child is too long
            if (this->left->balance_factor > 0)
            {
                rotate_left(tree, this->left); // Left-Right
            }
            rotate_right(tree, this);
        }

        // Right branch is too long
        else
        {
            // left side of right child is too long
            if (this->right->balance_factor < 0)
            {
                rotate_right(tree, this->right); // Right-Left
            }
            rotate_left(tree, this);
        }
    }
}

void rotate_left(avl_tree *tree, avl_node *this)
{
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

    // We need to update the parent of our former right child
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

    if (new_parent->balance_factor == 0)
    {
        this->balance_factor = +1;
        new_parent->balance_factor = -1;
    }
    else
    {
        this->balance_factor = 0;
        new_parent->balance_factor = 0;
    }
}

void rotate_right(avl_tree *tree, avl_node *this)
{
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

    // We need to update the parent of our former left child
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

    // Update the balance factors accordingly.

    if (new_parent->balance_factor == 0)
    {
        this->balance_factor = -1;
        new_parent->balance_factor = +1;
    }
    else
    {
        this->balance_factor = 0;
        new_parent->balance_factor = 0;
    }
}

int node_display_size = 10;
int buffer_size;

int calc_node_depth_recurse(avl_node *node)
{
    if (node == NULL)
    {
        return 0;
    }

    int dleft = calc_node_depth_recurse(node->left);
    int dright = calc_node_depth_recurse(node->right);

    return 1 + (dleft > dright ? dleft : dright);
}

int calc_tree_depth(avl_tree *tree)
{
    return calc_node_depth_recurse(tree->root);
}

void print_recurse(avl_tree *tree, avl_node *node, int row, int pos, int size, char **out)
{
    if (node == NULL)
    {
        return;
    }

    int my_index = buffer_size / (size + 1) * pos;

    char *side = "root";
    char s = 'R';
    if (node->parent != NULL)
    {
        side = node == node->parent->left ? "left" : "right";
        s = node == node->parent->left ? 'l' : 'r';
    }

    char tmp[node_display_size];
    sprintf(tmp, "%d:%c:%d", (int)(node->data), s, node->balance_factor);

    // avoid the \0 of snprintf
    for (int i = 0; i < node_display_size && tmp[i] != '\0'; i++)
    {
        out[row][my_index + i] = tmp[i];
    }

    print_recurse(tree, node->left, row + 1, pos * 2 - 1, size * 2, out);
    print_recurse(tree, node->right, row + 1, pos * 2, size * 2, out);
}

void pretty_print(avl_tree *tree)
{
    int tree_height = calc_tree_depth(tree);
    buffer_size = ceil(pow(2, tree_height) * node_display_size);

    printf("--= T = R = E = E =-- HEIGHT: %d (buffer_size: %d)\n", tree_height, buffer_size);

    char **out = (char **)calloc(tree_height, sizeof(char *));
    for (int i = 0; i <= tree_height; i++)
    {
        out[i] = (char *)calloc(buffer_size, sizeof(char));

        int j;
        for (j = 0; j < buffer_size; j++)
        {
            out[i][j] = ' ';
        }
        out[i][j - 1] = '\0';
    }

    print_recurse(tree, tree->root, 0, 1, 1, out);

    for (int i = 0; i <= tree_height; i++)
    {
        printf("%s\n", out[i]);
    }
}
