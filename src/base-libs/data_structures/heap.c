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
    h->Q = queue_new();
    return h;
}

void *heap_peek(heap *h) {
    return h->root->item;
}

heap *heap_push(heap *h, void *item) {
    heap_node *n = heap_node_new();
    n->item = item;
    n->size = 0;

    if (h->root == NULL) {
        h->root = n;
        h->size = 1;
        return h;
    } else {
        heap *x = heap_new(h->compare);
        x->root = n;
        x->size = 1;
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
    // TODO: Make use of the active records to achieve O(1)
    list_node *child = x->root->children->head;
    while (child != NULL) {
        ((heap_node *)(child->value))->is_active = 0;
        child = child->next;
    }

    // u->key < v->key
    heap_node *u, *v;
    heap *lesser_key_heap, *larger_key_heap;
    if (x->compare(x->root->item, y->root->item) < 0) {
        u = x->root;
        v = y->root;
        lesser_key_heap = x;
        larger_key_heap = y;
    } else {
        u = y->root;
        v = x->root;
        lesser_key_heap = y;
        larger_key_heap = x;
    }

    // O(1) Since v is a root node with no parent
    // Make v a child of u
    link(v, u);

    // O(1) Since these are all just pointer operations
    // update the queue Q, set it to Qx + v + Qy
    queue *Q = x->Q;
    v->position_in_q = queue_add(Q, v);
    queue_merge_into(Q, y->Q);
    lesser_key_heap->Q = Q;

    // Discard the irrelevant heap & update lesser_key_heap size
    lesser_key_heap->size += larger_key_heap->size;
    free(larger_key_heap);

    // Do an active root reduction and a root degree reduction to the extent possible.
    
    // TODO: active_root_reduction(lesser_key_heap, )
    root_degree_reduction(lesser_key_heap);

    return lesser_key_heap;
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
    heap_node *z = h->root;
    if (h->compare(x->item, z->item) < 0) {
        void *tmp = x->item;
        x->item = z->item;
        z->item = tmp;
    }

    // Let y be the parent of x
    heap_node *y = x->parent;

    // Make x a child of the root
    link(x, h->root);

    // If x was an active node but not an active root
    if (x->is_active && y->is_active) {
        // x becomes an active root with loss zero and the rank of y is decreased by one
        x->is_active = 1;
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

    // TODO: six active root reductions

    int rdr_count = 0;
    while(root_degree_reduction(h) && rdr_count < 4) {
        rdr_count++;
    }
}

void *heap_pop(heap *h) {

    // Return NULL if heap is empty.
    if (h->root == NULL) {
        return NULL;
    }

    // Return root if single node heap.
    if (h->root->children->size == 0) {
        void *item = h->root->item;
        heap_node_destroy(h->root);
        h->root = NULL;
        h->size = 0;
        return item;
    }

    // O(R) == O(log(N))
    // Find the node x of minimum key among the children of the root.
    list_node *list_el_x = h->root->children->head;
    heap_node *x = (heap_node *)(list_el_x->value);

    list_node *tmp = list_el_x->next;
    while (tmp != NULL) {
        heap_node *tmp_node = (heap_node *)(tmp->value);
        if (h->compare(tmp_node->item, x->item) < 0) {
            list_el_x = tmp;
            x = tmp_node;
        }
        tmp = tmp->next;
    }

    // O(1)
    // Delete the min child from the root's children's list
    list_del_node(h->root->children, list_el_x);

    // If x is active then make x passive and all active children of x become active roots.
    if (x->is_active) {
        x->is_active = 0; // TODO: Use active records
    }

    // O(R) == O(log(N))
    // Make each of the other children of the root a child of x.
    while (h->root->children->head != NULL) {
        heap_node *child_node = h->root->children->head->value;
        
        if (is_linkable(child_node)) {
            child_node->relative_position_to_siblings = list_append(x->children, child_node);
        } else {
            child_node->relative_position_to_siblings = list_prepend(x->children, child_node);
        }

        child_node->parent = x;
        list_del_first(h->root->children);
    }

    // Make the passive linkable children of x the rightmost children of x.
    // TODO: Is this condition be already satisfied by the prepend vs append logic above?

    // O(1)
    // Remove x from Q
    queue_remove_node(h->Q, x->position_in_q);

    // Destroy the root node and set it to x
    void *item = h->root->item;
    heap_node_destroy(h->root);
    h->root = x;
    h->size--;

    // TODO Repeat twice: move the front node y on Q to the back; link the two rightmost children of y to x, if they are passive.

    // TODO Do a loss reduction if possible.

    // TODO Do active root reductions and root degree reductions in any order until none of either is possible.

    return item;
}

void heap_destroy(heap *h) {
    queue_del(h->Q);
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
        list_del_node(x->parent->children, x->relative_position_to_siblings);
    }

    if (x->is_active) {
        // Prepend to y's child list
        x->relative_position_to_siblings = list_prepend(y->children, x);
    } else {
        // Append to y's child list
        x->relative_position_to_siblings = list_append(y->children, x);
    }
    x->parent = y;
    y->size = x->size + 1;
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
        link(rightmost_child, h->root); // Make z child of the root
    }
}

/**
 * In this transform both x and y change from being passive to active with loss
 * zero, both get one more child, and x becomes a new active root. The degree
 * of the root decreases by two and the number of active roots increases by one.
 */
char root_degree_reduction(heap *h) {
    // Will not be able to find the three rightmost linkable children if there are less than 3 children
    if (h->root->children->size < 3) {
        return 0;
    }

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
    list_del_node(h->root->children, x->relative_position_to_siblings);
    x->relative_position_to_siblings = list_prepend(h->root->children, x);

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

    z->rank--;
    if (!is_active_root(z)) {
        z->loss++;
    }
}
