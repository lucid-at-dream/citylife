#include "heap.h"
#include "heap_checks.h"
#include "heap_structs.h"

#include <stdlib.h>

void increase_node_rank(heap *h, heap_node *n)
{
    if (!is_active(n))
    {
        return; // Not Applicable
    }

    if (is_active_root(n) || n->loss > 0)
    {
        // Node rank points to a fix-list node.
        // TODO: How do we handle this scenario?
        return; // TODO: Fix-List
    }

    if (n->rank == NULL)
    {
        // Unexpected! An active node with loss <= 0 should be pointing to a rank record.
        return;
    }

    rank_list_record *current_rank = (rank_list_record *)(n->rank->value);
    current_rank->ref_count--;

    if (n->rank->prev == NULL)
    {
        // There's no rank node for the rank we want, must be created
        new_rank_list_record(h);
    }

    list_node *target_rank = n->rank->prev;
    ((rank_list_record *)(target_rank->value))->ref_count += 1;
    n->rank = target_rank;
}

void decrease_node_rank(heap *h, heap_node *n)
{
    if (!is_active(n))
    {
        return; // Not Applicable
    }

    if (is_active_root(n) || n->loss > 0)
    {
        // Node rank points to a fix-list node.
        // TODO: How do we handle this scenario?
        return; // TODO: Fix-List
    }

    if (n->rank == NULL)
    {
        // Unexpected! An active node with loss <= 0 should be pointing to a rank record.
        return;
    }

    if (n->rank->next == NULL)
    {
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

void set_node_rank(heap *h, heap_node *n, int rank_value)
{
    char active = is_active(n);

    if (is_active_root(n) || (active && n->loss > 0))
    {
        // Node rank points to a fix-list node.
        // TODO: How do we handle this scenario?
        // TODO: Fix-List
    }
    else if (active)
    {
        // Then n->rank should be pointing to a rank list node.
        rank_list_record *old_rank_node = n->rank->value;
        old_rank_node->ref_count--;
    }

    n->rank = h->rank_list->tail; // Set the node's rank to 0;

    if (n->rank == NULL)
    {
        // Error. This should never happen.
        return;
    }

    for (int i = 0; i < rank_value; i++)
    { // Increase node's rank as needed.
        if (n->rank->prev == NULL)
        {
            // There's no rank node for the rank we want, must be created
            new_rank_list_record(h);
        }
        n->rank = n->rank->prev;
    }
    rank_list_record *new_rank_node = n->rank->value;
    new_rank_node->ref_count++;
}

void new_rank_list_record(heap *h)
{
    rank_list_record *new_rank = calloc(1, sizeof(rank_list_record));
    new_rank->active_roots = NULL; // TODO: Fix-List
    new_rank->loss = NULL; // TODO: Fix-List
    new_rank->ref_count = 0;
    list_prepend(h->rank_list, new_rank);
}
