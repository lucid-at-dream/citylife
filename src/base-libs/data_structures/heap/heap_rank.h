#pragma once

#include "heap.h"

void increase_node_rank(heap *h, heap_node *n);
void decrease_node_rank(heap *h, heap_node *n);
void set_node_rank(heap *h, heap_node *n, int rank_value);
void new_rank_list_record(heap *h);
