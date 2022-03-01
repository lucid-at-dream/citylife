#include "heap.h"

#include <stdlib.h>
#include <stdio.h>

// TODO: Maintain fix-list up to date
// TODO: Review transformations and add the article's text as comments
// TODO: Apply transformations when possible
// TODO: Implement an heap push operation without having to meld (optimization)

heap_node *heap_node_new();
void heap_node_destroy(heap_node *n);

// Node property checks
char is_linkable(heap_node *x);
char is_active_root(heap_node *x);
char is_active(heap_node *x);

// Transformations
void link(heap_node *x, heap_node *y);
void active_root_reduction(heap *h, heap_node *active_root_x, heap_node *active_root_y);
char root_degree_reduction(heap *h);
void one_node_loss_reduction(heap *h, heap_node *x);
void two_node_loss_reduction(heap *h, heap_node *a, heap_node *b);

// Node rank operations
void increase_node_rank(heap *h, heap_node *n);
void decrease_node_rank(heap *h, heap_node *n);
void set_node_rank(heap *h, heap_node *n, int rank_value);
void new_rank_list_record(heap *h);

heap *heap_new(int (*compare)(const void *a, const void *b)) {
    heap *h = (heap *)calloc(1, sizeof(heap));
    h->compare = compare;
    h->root = NULL;

    h->Q = queue_new();
    
    h->active_record = (active_record *)calloc(1, sizeof(active_record));
    h->active_record->is_active = 1;

    h->rank_list = list_new();
    new_rank_list_record(h); // Create rank 0 record

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

/**
 * A passive node is linkable if all its children are passive.
 */
char is_linkable(heap_node *x) {
    if (x == NULL) {
        return 0;
    }

    if (!is_active(x)) {
        if (x->children == NULL) {
            return 1;
        }

        list_node *child = x->children->head;
        while (child != NULL) {
            if (is_active(child->value)) {
                return 0;
            }
            child = child->next;
        }
        return 1;
    }
    return 0;
}

char is_active_root(heap_node *x) {
    return is_active(x) && !is_active(x->parent);
}

char is_active(heap_node *x) {
    return x != NULL && x->activity != NULL && x->activity->is_active;
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

    if (is_active(x)) {
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
 * O(1)
 * 
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
        } else {
            break;
        }
        child = child->prev;
    }

    // Unable to apply transformation
    if (idx < 3) {
        return 0;
    }

    // sort them by key: x.key < y.key < z.key

    // if a[0] > a[1], switch them
    if (h->compare(three_linkable_rightmost_nodes[0]->item, three_linkable_rightmost_nodes[1]->item) > 0) {
        heap_node *tmp = three_linkable_rightmost_nodes[0];
        three_linkable_rightmost_nodes[0] = three_linkable_rightmost_nodes[1];
        three_linkable_rightmost_nodes[1] = tmp;
    }

    // if a[0] > a[2], make a[0] = a[2]; a[1] = a[0]; a[2] = a[1]
    if (h->compare(three_linkable_rightmost_nodes[0]->item, three_linkable_rightmost_nodes[2]->item) > 0) {
        heap_node *tmp0 = three_linkable_rightmost_nodes[0];
        three_linkable_rightmost_nodes[0] = three_linkable_rightmost_nodes[2];

        heap_node *tmp1 = three_linkable_rightmost_nodes[1];
        three_linkable_rightmost_nodes[1] = tmp0;
        three_linkable_rightmost_nodes[2] = tmp1;
    }

    // if a[1] > a[2], switch them
    if (h->compare(three_linkable_rightmost_nodes[1]->item, three_linkable_rightmost_nodes[2]->item) > 0) {
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
    x->activity = h->active_record; h->active_record->ref_count += 1;
    y->activity = h->active_record; h->active_record->ref_count += 1;

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
    increase_node_rank(h, lesser_root);

    // Get rightmost child of x (call it z).
    heap_node *rightmost_child = (heap_node *)list_get_last(lesser_root->children);

    // If z exists and is passive
    if (rightmost_child != NULL && !is_active(rightmost_child)) {
        link(rightmost_child, h->root); // Make z child of the root
    }
}

/**
 * The loss of x decreases by at least two, the total loss is decreased by at least one.
 */
void one_node_loss_reduction(heap *h, heap_node *x) {
    // applies if x->is_active && x->loss >= 2
    if (!is_active(x) || x->loss < 2) {
        return; // Nothing to do.
    }

    heap_node *y = x->parent;

    // link x to the root and make it and active root with loss zero
    link(x, h->root);
    x->loss = 0;

    // decrease y rank by one
    decrease_node_rank(h, y);

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
    increase_node_rank(h, x);
    x->loss = y->loss = 0;

    decrease_node_rank(h, z);
    if (!is_active_root(z)) {
        z->loss++;
    }
}

void increase_node_rank(heap *h, heap_node *n) {

    if (!is_active(n)) {
        return; // Not Applicable
    }

    if (is_active_root(n) || n->loss > 0) {
        // Node rank points to a fix-list node.
        // TODO: How do we handle this scenario?
        return; // TODO: Fix-List
    }
    
    if (n->rank == NULL) {
        // Unexpected! An active node with loss <= 0 should be pointing to a rank record.
        return;
    }

    rank_list_record *current_rank = (rank_list_record *)(n->rank->value);
    current_rank->ref_count--;

    if (n->rank->prev == NULL) {
        // There's no rank node for the rank we want, must be created
        new_rank_list_record(h);
    }

    list_node *target_rank = n->rank->prev;
    ((rank_list_record *)(target_rank->value))->ref_count += 1;
    n->rank = target_rank;
}

void decrease_node_rank(heap *h, heap_node *n) {

    if (!is_active(n)) {
        return; // Not Applicable
    }

    if (is_active_root(n) || n->loss > 0) {
        // Node rank points to a fix-list node.
        // TODO: How do we handle this scenario?
        return; // TODO: Fix-List
    }

    if (n->rank == NULL) {
        // Unexpected! An active node with loss <= 0 should be pointing to a rank record.
        return;
    }

    if (n->rank->next == NULL) {
        // Node is already at rank 0 and cannot be decreased any further
        return;
    }

    list_node *old_rank = n->rank;
    rank_list_record *old_rank_node = old_rank->value;
    old_rank_node->ref_count--;

    n->rank = n->rank->next;
    rank_list_record *new_rank_node = n->rank->value;
    new_rank_node->ref_count++;
}

void set_node_rank(heap *h, heap_node *n, int rank_value) {

    char active = is_active(n);

    if (is_active_root(n) || (active && n->loss > 0)) {
        // Node rank points to a fix-list node.
        // TODO: How do we handle this scenario?
        // TODO: Fix-List

    } else if (active){
        // Then n->rank should be pointing to a rank list node.
        rank_list_record *old_rank_node = n->rank->value;
        old_rank_node->ref_count--;
    }

    n->rank = h->rank_list->tail; // Set the node's rank to 0;

    if (n->rank == NULL) {
        // Error. This should never happen.
        return;
    }

    for (int i = 0; i < rank_value; i++) { // Increase node's rank as needed.
        if (n->rank->prev == NULL) {
            // There's no rank node for the rank we want, must be created
            new_rank_list_record(h);
        }        
        n->rank = n->rank->prev;
    }
    rank_list_record *new_rank_node = n->rank->value;
    new_rank_node->ref_count++;
}

void new_rank_list_record(heap *h) {
    rank_list_record *new_rank = calloc(1, sizeof(rank_list_record));
    new_rank->active_roots = NULL; // TODO: Fix-List
    new_rank->loss = NULL; // TODO: Fix-List
    new_rank->ref_count = 0;
    list_prepend(h->rank_list, new_rank);
}
