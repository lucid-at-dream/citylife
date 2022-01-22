#include "heap.h"

#include <stdlib.h>

void heap_node_destroy(heap_node *n);
heap_node *heap_node_new();

heap *heap_new() {
    return (heap *)calloc(1, sizeof(heap));
}

heap_node *heap_node_new() {
    heap_node *n = (heap_node *)calloc(1, sizeof(heap_node));
    n->children = list_new();
    return n;
}

void heap_push(heap *h, void *item) {
    if (h->root == NULL) {
        h->root = heap_node_new();
    }
    h->root->item = item;
}

void *heap_pop(heap *h) {
    void *tmp = h->root->item;
    h->root->item = NULL;
    return tmp;
}

void *heap_peek(heap *h) {
    return h->root->item;
}

void heap_destroy(heap *h) {
    if (h->root) {
        heap_node_destroy(h->root);
    }
    free(h);
}

void heap_node_destroy(heap_node *n) {
    list_destroy(n->children);
    free(n);
}

#pragma region implementation_details

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

#pragma endregion implementation_details