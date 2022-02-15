#include "heap.h"

#include <stdlib.h>
#include <stdio.h>

heap_node *heap_node_new();
void heap_node_destroy(heap_node *n);

// Node property checks
char is_linkable(heap_node *x);
char is_active_root(heap_node *x);

// Transformations
void link(heap_node *x, heap_node *y);
void active_root_reduction(heap *h, heap_node *active_root_x, heap_node *active_root_y);
char root_degree_reduction(heap *h);
void one_node_loss_reduction(heap *h, heap_node *x);
void two_node_loss_reduction(heap *h, heap_node *a, heap_node *b);

heap *heap_new(int (*compare)(const void *a, const void *b)) {
    heap *h = (heap *)calloc(1, sizeof(heap));
    h->compare = compare;
    return h;
}

void *heap_peek(heap *h) {
    return h->root->item;
}

heap *heap_push(heap *h, void *item) {
    heap_node *n = heap_node_new();
    n->item = item;

    if (h->root == NULL) {
        h->root = n;
        return h;
    } else {
        heap *x = heap_new(h->compare);
        x->root = n;
        return heap_meld(h, x);
    }
}

heap *heap_meld(heap *h1, heap *h2) {
    // x->size <= y->size
    heap *x, *y;
    if (h1->root->size <= h2->root->size) {
        x = h1;
        y = h2;
    } else {
        x = h2;
        y = h1;
    }

    // Make all nodes in the tree rooted at x passive
    // TODO: Do this implicitly, as described in Section 6, so that it takes O81) time.)
    list_node *child = x->root->children->head;
    while (child != NULL) {
        ((heap_node *)(child->value))->is_active = 0;
        child = child->next;
    }

    // u->key < v->key
    heap_node *u, *v;
    heap *final_heap, *discarded_heap;
    if (y->compare(x->root->item, y->root->item) < 0) {
        u = x->root;
        v = y->root;
        final_heap = x;
        discarded_heap = y;
    } else {
        u = y->root;
        v = x->root;
        final_heap = y;
        discarded_heap = x;
    }

    list_append(u->children, v);
    v->parent = u;

    free(discarded_heap);

    // TODO: update the queue Q

    // TODO: find active roots
    // active_root_reduction(final_heap, )

    // Do a root degree reduction to the extent possible
    int transformation_succeeded = 1;
    while (transformation_succeeded > 0) {
        transformation_succeeded = root_degree_reduction(final_heap);
    }

    return final_heap;
}

void heap_decrease_key(heap *h, heap_node *x, void *new_item) {
    if (h->compare(x->item, new_item) < 0) {
        // Increasing the key is not supported
        return;
    }

    // Begin by decrasing the key of the item
    x->item = new_item;

    // If x is the root we are done
    if (h->root == x) {
        return;
    }

    // if x.key < z.key swap the items in x and z
    // TODO: Notice how key and item are two separate things, requires changing the struct =/
    heap_node *z = h->root;
    if (h->compare(x->item, z->item) < 0) {
        void *tmp = x->item;
        x->item = z->item;
        z->item = tmp;
    }

    heap_node *y = x->parent;

    // Make x a child of the root
    list_append(z->children, x);
    x->parent = z;

    // If x was an active node but not an active root
    if (x->is_active && x->parent->is_active) {
        // x becomes an active root with loss zero and the rank of y is decreased by one
        x->loss = 0;
        y->rank--;
    }

    // If y is active but not an active root
    if (y->is_active && y->parent->is_active) {
        // the loss of y is increased by one.
        y->loss++;
    }

    // TODO:
    // Do a loss reducion if possible.

    // TODO:
    // Finally, do six active root reductions and four root degree reductions to the extent possible.

    root_degree_reduction(h);
}

void *heap_pop(heap *h) {

    // Find the node x of minimum key mong the children of the root.
    list_node *list_el_x = h->root->children->head;

    if (list_el_x == NULL) {
        void *item = h->root->item;
        heap_node_destroy(h->root);
        h->root = NULL;
        return item;
    }

    list_node *tmp = list_el_x->next;
    while(tmp != NULL) {
        if (h->compare(tmp->value, list_el_x->value) < 0) {
            list_el_x = tmp;
        }
        tmp = tmp->next;
    }

    heap_node *x = (heap_node *)(list_el_x->value);
    list_del_element(h->root->children, list_el_x->value);

    // If x is active then make x passive and all active children of x become active roots.
    if (x->is_active) {
        x->is_active = 0;
    }

    // Make each of the other children of z a child of x.

    while(h->root->children->head != NULL) {
        heap_node *child_node = h->root->children->head->value;
        if (is_linkable(child_node)) {
            list_append(x->children, child_node);
        } else {
            list_prepend(x->children, child_node);
        }
        child_node->parent = x;
        list_del_first(h->root->children);
    }

    // Make the passive linkable children of x the rightmost children of x.
    list_node *x_child = x->children->head;
    list_node *first_linkable_node_found = NULL;
    while (x_child && x_child->next != NULL) {
        if (is_linkable(x_child->value)) {

            if (!first_linkable_node_found) {
                first_linkable_node_found = x_child;
            } else {
                if (first_linkable_node_found == x_child) {
                    break;
                }
            }

            list_node *prev = x_child->prev;
            list_node *next = x_child->next;

            if (prev != NULL) {
                x_child->prev->next = x_child->next;
            }

            // Move linkable child to the rightmost position of the list
            x_child->next->prev = x_child->prev;
            x->children->tail->next = x_child;
            x_child->prev = x->children->tail;
            x_child->next = NULL;
            x->children->tail = x_child;

            x_child = next;
        } else {
            x_child = x_child->next;
        }
    }

    // Remove x from Q

    // Destroy z
    void *item = h->root->item;
    heap_node_destroy(h->root);
    h->root = x;

    // Repeat twice: move the front node y on Q to the back; link the two rightmost children of y to x, if they are passive.

    // Do a loss reduction if possible.

    // Do active root reductions and root degree reductions in any order until none of either is possible.

    return item;
}

void heap_destroy(heap *h) {
    if (h->root) {
        heap_node_destroy(h->root);
    }
    free(h);
}

heap_node *heap_node_new() {
    heap_node *n = (heap_node *)calloc(1, sizeof(heap_node));
    n->children = list_new();
    return n;
}

void heap_node_destroy(heap_node *n) {
    list_node *child = n->children->head;
    while (child != NULL) {
        heap_node_destroy(child->value);
        child = child->next;
    }

    list_destroy(n->children);
    free(n);
}

/**
 * A passive node is linkable if all its children are passive.
 */
char is_linkable(heap_node *x) {
    if (!x->is_active) {
        list_node *child = x->children->head;
        while (child != NULL) {
            if (((heap_node *)(child->value))->is_active) {
                return 0;
            }
            child = child->next;
        }
        return 1;
    }
    return 0;
}

char is_active_root(heap_node *x) {
    return x->is_active && !x->parent->is_active;
}

/**
 * The basic transformation is to link a node x and its subtree below another
 * node y, by removing x from the child list of its current parent and making x
 * a child of y. If x is active it is made the leftmost child of y; if x is 
 * passive it is made the rightmost child of y.
 */
void link(heap_node *x, heap_node *y) {
    // remove x from its parent's child list
    if (x->parent != NULL) {
        list_del_element(x->parent->children, x);
    }

    if (x->is_active) {
        // Prepend to y's child list
        list_prepend(y->children, x);
    } else {
        // Append to y's child list
        list_append(y->children, x);
    }
    x->parent = y;
}

/**
 * In this transform the number of active roots is decreased by one and the
 * degree of the root possibly increased by one. 
 */
void active_root_reduction(heap *h, heap_node *active_root_x, heap_node *active_root_y) {
    // Compare root keys
    int rel = h->compare(active_root_x->item, active_root_y->item);
    heap_node *lesser_root, *larger_root;
    if (rel < 0) {
        lesser_root = active_root_x;
        larger_root = active_root_y;
    } else {
        lesser_root = active_root_y;
        larger_root = active_root_x;
    }

    link(larger_root, lesser_root);
    lesser_root->rank++;

    // Get rightmost child of x (call it z).
    heap_node *rightmost_child = (heap_node *)list_get_last(lesser_root->children);

    // If z exists and is passive
    if (rightmost_child != NULL && !rightmost_child->is_active) {
        list_append(h->root->children, rightmost_child); // Make z child of the root
        list_del_last(lesser_root->children); // Remove it from x's child list
    }
}

/**
 * In this transform both x and y change from being passive to active with loss
 * zero, both get one more child, and x becomes a new active root. The degree
 * of the root decreases by two and the number of active roots increases by one.
 */
char root_degree_reduction(heap *h) {
    // Find the three rightmost passive linkable children of the root.
    heap_node *three_linkable_rightmost_nodes[3];

    int idx = 0;
    list_node *child = h->root->children->tail;
    while (child != NULL && idx < 3) {
        if (is_linkable(child->value)) {
            three_linkable_rightmost_nodes[idx] = child->value;
            idx++;
        }
        child = child->next;
    }

    // Unable to apply transformation
    if (idx < 3) {
        return 0;
    }

    // sort them by key: x.key < y.key < z.key
    qsort(three_linkable_rightmost_nodes, 3, sizeof(heap_node *), h->compare); // TODO: This is comparing the nodes, not the items
    heap_node *x = three_linkable_rightmost_nodes[0], *y = three_linkable_rightmost_nodes[1], *z = three_linkable_rightmost_nodes[2];

    // Mark x and y as active
    x->is_active = y->is_active = 1;

    // Link z to y and link y to x
    link(z, y);
    link(y, x);

    // Make x the leftmost child of the root
    list_del_element(h->root->children, x);
    list_prepend(h->root->children, x);

    // Assign both x and y loss zero, and rank one and zero respectively
    x->loss = y->loss = 0;
    x->rank = 1;
    y->rank = 0;

    return 1;
}

/**
 * The loss of x decreases by at least two, the total loss is decreased by at least one.
 */
void one_node_loss_reduction(heap *h, heap_node *x) {
    // applies if x->is_active && x->loss >= 2

    heap_node *y = x->parent;

    // link x to the root and make it and active root with loss zero
    link(x, h->root);
    x->is_active = 1;
    x->loss = 0;

    // decrease y rank by one
    y->rank--;

    // if y is not an active root the loss of y is increased by one.
    if (!is_active_root(y)) {
        y->loss++;
    }
}

void two_node_loss_reduction(heap *h, heap_node *a, heap_node *b) {
    // applies when x->is_active && y->is_active && x->rank == y->rank == 1

    // x.key < y.key
    heap_node *x, *y;
    if (h->compare(a->item, b->item) < 0) {
        x = a;
        y = b;
    } else {
        x = b;
        y = a;
    }

    heap_node *z = y->parent;

    link(y, x);
    x->rank++;
    x->loss = y->loss = 0;

    z->degree--;
    z->rank--;
    if (!is_active_root(z)) {
        z->loss++;
    }
}
