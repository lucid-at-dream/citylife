#pragma once

#include "heap.h"

typedef struct _active_record active_record;
typedef struct _rank_list_record rank_list_record;

struct _active_record
{
    // Nodes pointing to this record are active if and only if the flag is true.
    char is_active;

    // The number of nodes pointing to the record. If flag is false and ref-count is 0, then the node can be free'd
    int ref_count;
};

struct _rank_list_record
{
    // The rank to which this rank record refers to.
    int value;

    // A pointer to a record in the fix-list for an active node with rank r and positive loss. NULL if no such node exists.
    list_node *loss;

    // A pointer to a record in the fix-list for an active root with rank r. NULL if no such node exists.
    list_node *active_roots;

    // The number of node records and fix-list records pointing to this record.
    // If the leftmost record on the rank-list gets a ref-count = 0, then the record is deleted from the rank-list and is freed.
    int ref_count;
};

struct _heap_node
{
    // The item being stored in this heap node
    void *item;

    // If the node is passive, the value of the pointer is not defined.
    // If the node is an active root or an active node with positive loss (i.e. it is on the fix-list), rank
    // points to the corresponding record in the fix-list.
    // Otherwise rank points to the record in the rank-list corresponding to the rank of the node.
    // (The cases can be distinguished using the active field of the node and the parent together with the loss field).
    list_node *rank;

    // Number of nodes below this one
    int size;

    // Total loss of an heap is the sum of the loss over all active nodes
    unsigned loss; // TODO: Is this being correctly updated?

    // is active or passive?
    active_record *activity;

    // The parent of the node
    struct _heap_node *parent;

    // The relative position of the node with respect to its siblings in the parent node.
    list_node *relative_position_to_siblings;

    // The node's position in heap->Q
    queue_item *position_in_q;

    // List of the children of this node
    list *children;
};

struct _heap
{
    // Total number of nodes in the heap
    int size;

    // Root node of the heap
    heap_node *root;

    // Returns -1 if a < b, 1 if a > b and 0 if they're equal
    int (*compare)(const void *a, const void *b);

    // A pointer to the active record shared by all active odes in the heap (one distinct active record for each heap).
    active_record *active_record;

    // All nodes except the root are kept in this queue
    queue *Q;

    // A pointer to the rightmost record in the rank-list.
    list *rank_list;

    /* We maintain all active nodes that potentially could participate in one of the transformations from 
     * Section 3 (i.e. active roots and active nodes with positive loss) in a list called fix-list. Each node
     * with rank k on the fixlist points to node k of the rank-list. The fix-list is divided left-to-right 
     * into four parts (1-4), where Parts 1-2 contain active roots and Parts 3-4 contain nodes with positive loss.
     * 
     * Part 1 
     * contains the active roots of active-root transformable ranks. All the active roots of the same rank
     * are adjacent, and one of the nodes has a pointer from the corresponding node of the rank-list.
     * 
     * Part 2 
     * contains the remaining active roots. Each node has a pointer from the corresponding node of the rank-list. 
     * 
     * Part 3 
     * contains active nodes with loss one and a rank that is not loss-transformable. Each node of this part has
     * a pointer from the corresponding node of the rank-list. 
     * 
     * Part 4
     * contains all the active nodes of loss transformable rank. As in Part 1, all nodes of equal rank are adjacent
     * and one of the nodes is pointed to by the corresponding node of the rank-list. Observe that for some ranks 
     * there may exist only one node in Part 4 (because if the loss of a node is at least two, its rank is loss-transformable).
     *
     * Notes:
     *  - We call a rank active-root transformable, if there are at least two active roots of that rank. 
     *  - We call a rank loss transformable, if the total loss of the nodes of that rank is at least two.
     */
    list *fix_list;

    // A pointer to the leftmost node in the fix-list with a positive loss, if such a node exists. Otherwise it is NULL.
    list_node *singles;
};
