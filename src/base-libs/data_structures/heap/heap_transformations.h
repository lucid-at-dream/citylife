#pragma once

#include "heap.h"

void link(heap_node *x, heap_node *y);
void active_root_reduction(heap *h, heap_node *active_root_x, heap_node *active_root_y);
char root_degree_reduction(heap *h);
void one_node_loss_reduction(heap *h, heap_node *x);
void two_node_loss_reduction(heap *h, heap_node *a, heap_node *b);