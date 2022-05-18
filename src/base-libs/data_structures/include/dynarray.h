#pragma once

typedef struct _dynarray
{
    int size;
    int capacity;
    void **data;
} dynarray;

dynarray *dynarray_new();
void dynarray_destroy(dynarray *);

void dynarray_add(dynarray *ar, void *el);
void dynarray_del(dynarray *ar, int idx);
void *dynarray_get(dynarray *ar, int idx);
void dynarray_set(dynarray *ar, int idx, void *el);