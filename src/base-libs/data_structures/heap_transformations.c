#include "heap.h"
#include "heap_checks.h"
#include "heap_rank.h"
#include "heap_structs.h"

#include <stdlib.h>

/**
 * The basic transformation is to link a node x and its subtree below another
 * node y, by removing x from the child list of its current parent and making x
 * a child of y. If x is active it is made the leftmost child of y; if x is 
 * passive it is made the rightmost child of y.
 */
void link(heap_node *x, heap_node *y)
{
    // remove x from its parent's child list
    if (x->parent != NULL)
    {
        list_del_node(x->parent->children, x->relative_position_to_siblings);
    }

    if (is_active(x))
    {
        // Prepend to y's child list
        x->relative_position_to_siblings = list_prepend(y->children, x);
    }
    else
    {
        // Append to y's child list
        x->relative_position_to_siblings = list_append(y->children, x);
    }
    x->parent = y;
    y->size = x->size + 1;
}

/**
 * O(1)
 * 
 * In this transform both x and y change from being passive to active with loss
 * zero, both get one more child, and x becomes a new active root. The degree
 * of the root decreases by two and the number of active roots increases by one.
 */
char root_degree_reduction(heap *h)
{
    // Will not be able to find the three rightmost linkable children if there are less than 3 children
    if (h->root->children->size < 3)
    {
        return 0;
    }

    // Find the three rightmost passive linkable children of the root.
    heap_node *three_linkable_rightmost_nodes[3];

    int idx = 0;
    list_node *child = h->root->children->tail;
    while (child != NULL && idx < 3)
    {
        if (is_linkable(child->value))
        {
            three_linkable_rightmost_nodes[idx] = child->value;
            idx++;
        }
        else
        {
            break;
        }
        child = child->prev;
    }

    // Unable to apply transformation
    if (idx < 3)
    {
        return 0;
    }

    // sort them by key: x.key < y.key < z.key

    // if a[0] > a[1], switch them
    if (h->compare(three_linkable_rightmost_nodes[0]->item, three_linkable_rightmost_nodes[1]->item) > 0)
    {
        heap_node *tmp = three_linkable_rightmost_nodes[0];
        three_linkable_rightmost_nodes[0] = three_linkable_rightmost_nodes[1];
        three_linkable_rightmost_nodes[1] = tmp;
    }

    // if a[0] > a[2], make a[0] = a[2]; a[1] = a[0]; a[2] = a[1]
    if (h->compare(three_linkable_rightmost_nodes[0]->item, three_linkable_rightmost_nodes[2]->item) > 0)
    {
        heap_node *tmp0 = three_linkable_rightmost_nodes[0];
        three_linkable_rightmost_nodes[0] = three_linkable_rightmost_nodes[2];

        heap_node *tmp1 = three_linkable_rightmost_nodes[1];
        three_linkable_rightmost_nodes[1] = tmp0;
        three_linkable_rightmost_nodes[2] = tmp1;
    }

    // if a[1] > a[2], switch them
    if (h->compare(three_linkable_rightmost_nodes[1]->item, three_linkable_rightmost_nodes[2]->item) > 0)
    {
        heap_node *tmp = three_linkable_rightmost_nodes[1];
        three_linkable_rightmost_nodes[1] = three_linkable_rightmost_nodes[2];
        three_linkable_rightmost_nodes[2] = tmp;
    }

    heap_node *x = three_linkable_rightmost_nodes[0], *y = three_linkable_rightmost_nodes[1], *z = three_linkable_rightmost_nodes[2];

    // Assign both x and y loss zero, and rank one and zero respectively
    x->loss = y->loss = 0;
    set_node_rank(h, x, 1);
    set_node_rank(h, y, 0);

    // Mark x and y as active
    x->activity = h->active_record;
    h->active_record->ref_count += 1;
    y->activity = h->active_record;
    h->active_record->ref_count += 1;

    // Link z to y and link y to x
    link(z, y);
    link(y, x);

    // Make x the leftmost child of the root
    list_del_node(h->root->children, x->relative_position_to_siblings);
    x->relative_position_to_siblings = list_prepend(h->root->children, x);

    return 1;
}

/**
 * In this transform the number of active roots is decreased by one and the
 * degree of the root possibly increased by one.
 */
void active_root_reduction(heap *h, heap_node *active_root_x, heap_node *active_root_y)
{
    // Compare root keys
    int rel = h->compare(active_root_x->item, active_root_y->item);
    heap_node *lesser_root, *larger_root;
    if (rel < 0)
    {
        lesser_root = active_root_x;
        larger_root = active_root_y;
    }
    else
    {
        lesser_root = active_root_y;
        larger_root = active_root_x;
    }

    link(larger_root, lesser_root);
    increase_node_rank(h, lesser_root);

    // Get rightmost child of x (call it z).
    heap_node *rightmost_child = (heap_node *)list_get_last(lesser_root->children);

    // If z exists and is passive
    if (rightmost_child != NULL && !is_active(rightmost_child))
    {
        link(rightmost_child, h->root); // Make z child of the root
    }
}

/**
 * The loss of x decreases by at least two, the total loss is decreased by at least one.
 */
void one_node_loss_reduction(heap *h, heap_node *x)
{
    // applies if x->is_active && x->loss >= 2
    if (!is_active(x) || x->loss < 2)
    {
        return; // Nothing to do.
    }

    heap_node *y = x->parent;

    // link x to the root and make it and active root with loss zero
    link(x, h->root);
    x->loss = 0;

    // decrease y rank by one
    decrease_node_rank(h, y);

    // if y is not an active root the loss of y is increased by one.
    if (!is_active_root(y))
    {
        y->loss++;
    }
}

void two_node_loss_reduction(heap *h, heap_node *a, heap_node *b)
{
    // applies when x->is_active && y->is_active && x->rank == y->rank == 1

    // x.key < y.key
    heap_node *x, *y;
    if (h->compare(a->item, b->item) < 0)
    {
        x = a;
        y = b;
    }
    else
    {
        x = b;
        y = a;
    }

    heap_node *z = y->parent;

    link(y, x);
    increase_node_rank(h, x);
    x->loss = y->loss = 0;

    decrease_node_rank(h, z);
    if (!is_active_root(z))
    {
        z->loss++;
    }
}
