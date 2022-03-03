#pragma once

#include "heap.h"

char validate_invariants(heap *h);

char is_linkable(heap_node *x);
char is_active_root(heap_node *x);
char is_active(heap_node *x);