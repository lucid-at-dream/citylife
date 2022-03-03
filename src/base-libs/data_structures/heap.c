#include "heap.h"
#include "heap_structs.h"
#include "heap_checks.h"
#include "heap_transformations.h"
#include "heap_rank.h"

#include <stdlib.h>
#include <stdio.h>

// TODO: Maintain fix-list up to date
// TODO: Review transformations and add the article's text as comments
// TODO: Apply transformations when possible
// TODO: Implement an heap push operation without having to meld (optimization)

heap_node *heap_node_new();
void heap_node_destroy(heap_node *n);

heap *heap_new(int (*compare)(const void *a, const void *b)) {
    heap *h = (heap *)calloc(1, sizeof(heap));
    h->compare = compare;
    h->root = NULL;

    h->Q = queue_new();
    
    h->active_record = (active_record *)calloc(1, sizeof(active_record));
    h->active_record->is_active = 1;

    h->rank_list = list_new();
    new_rank_list_record(h); // Create rank 0 record

    h->fix_list = list_new();

    return h;
}

void free_rank_list(void **record) {
    free(*record);
}

void heap_destroy(heap *h) {
    queue_del(h->Q);

    if (h->root != NULL) {
        heap_node_destroy(h->root);
    }

    if (h->active_record->ref_count <= 0) {
        free(h->active_record);
    }

    list_foreach(h->rank_list, free_rank_list);
    list_destroy(h->rank_list);

    list_destroy(h->fix_list);

    free(h);
}

heap_node *heap_node_new() {
    heap_node *n = (heap_node *)calloc(1, sizeof(heap_node));
    n->children = list_new();
    n->size = 0;
    n->activity = NULL;
    return n;
}

void heap_node_destroy(heap_node *n) {
    
    list_node *child = n->children->head;
    while (child != NULL) {
        heap_node_destroy(child->value);
        child = child->next;
    }

    list_destroy(n->children);
    
    if (is_active(n)) {
        n->activity->ref_count -= 1;
    }
    free(n);
}

void *heap_peek(heap *h) {
    return h->root->item;
}

heap *heap_push(heap *h, void *item, heap_node **node) {
    heap_node *n = heap_node_new();
    n->item = item;
    *node = n;

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
    x->active_record->is_active = 0;
    if (x->active_record->ref_count == 0) {
        free(x->active_record);
        x->active_record = y->active_record;
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

    // Update lesser_key_heap size
    lesser_key_heap->size += larger_key_heap->size;
    
    // Discard the irrelevant heap
    list_foreach(larger_key_heap->rank_list, free_rank_list);
    list_destroy(larger_key_heap->rank_list);
    list_destroy(larger_key_heap->fix_list);
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
    // x becomes an active root with loss zero and the rank of y is decreased by one
    // Note: since x is active and the root (its new parent) is always passive, x naturally becomes an active root.
    if (is_active(x) && is_active(y)) {
        x->loss = 0;
        decrease_node_rank(h, y);
    }

    // If y is active but not an active root
    if (is_active(y) && !is_active_root(y)) {
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
    if (is_active(x)) {
        // Make x passive
        // Note: By making x passive, all active children of x become active roots.
        active_record *a_record = x->activity;
        a_record->ref_count -= 1;
        if (!a_record->is_active && a_record->ref_count == 0) {
            free(a_record);
        }
        x->activity = NULL;
    } else {
    }

    // O(R) == O(log(N))
    // Make each of the other children of the root a child of x.
    // Make the passive linkable children of x the rightmost children of x. // TODO: Is this condition be already satisfied by the prepend vs append logic above?
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
