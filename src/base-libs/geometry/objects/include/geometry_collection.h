#pragma once

#include "dynarray.h"

typedef struct _geometry_collection {
    dynarray *geometry_list;
} geometry_collection;

geometry_collection *geometry_collection_new();
