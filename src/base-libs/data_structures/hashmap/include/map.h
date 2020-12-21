#pragma once

typedef struct _map_entry {
    char *key;
    void *value;
} map_entry;

typedef struct _bucket {
    map_entry entry;
    struct _bucket *next;
} bucket;

typedef struct _bucket_list {
    bucket *begin;
} bucket_list;

typedef struct _map {
    int capacity;
    int size;
    bucket_list *table;
} map;

// Public features
map *map_new(int capacity);
void map_set(map *m, char *key, void *value);
void *map_get(map *m, char *key);
void map_del(map *m, char *key);

// Memory management
void map_destroy(map *m);
void map_destroy_dealloc(map *m, char dealloc_keys, char dealloc_vals);

// Debug
void map_display(map *m);
