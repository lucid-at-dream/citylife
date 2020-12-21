#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "map.h"
#include "logger.h"

// private destructors
void bucket_list_destroy(bucket *b, char dealloc_keys, char dealloc_vals);

// private map operations
void resize_map(map *m, unsigned int new_size);

// private auxiliar functions
char should_resize(map *m);
unsigned get_index(map *m, char *key);
unsigned int calc_next_resize(map *m);
void map_display(map *m);
unsigned calc_hash(const char *key);
unsigned int qhashmurmur3_32(const void *data, size_t nbytes);

// Function definitions
map *map_new(int capacity)
{
    debug("map.c: Creating new map with capacity %d", capacity);
    map *m = (map *)malloc(sizeof(map));
    m->capacity = capacity;
    m->size = 0;
    m->table = (bucket_list *)calloc(capacity, sizeof(bucket_list));
    return m;
}

bucket *bucket_new(char *key, void *value)
{
    map_entry entry = { key, value };
    bucket *new_bucket = (bucket *)calloc(1, sizeof(bucket));
    new_bucket->entry = entry;
    new_bucket->next = NULL;
    return new_bucket;
}

void map_set(map *m, char *key, void *value)
{
    debug("map.c: Setting value of key %s to %s", key, value);

    if (should_resize(m)) {
        resize_map(m, calc_next_resize(m));
    }

    int index = get_index(m, key);
    bucket_list *list = m->table + index;
    bucket *buck = list->begin;

    if (buck == NULL) {
        debug("map.c: Adding new entry to the map at index %d", index);
        m->size++;
        list->begin = bucket_new(key, value);
        return;
    }

    while (buck != NULL) {
        if (strcmp(buck->entry.key, key) == 0) {
            debug("map.c: Updating existing entry in the map at index %d", index);
            // TODO: Add a some flag to allow freeing existing values.
            // free(buck->entry.value);
            buck->entry.value = value;
            return;
        }

        if (buck->next == NULL) {
            debug("map.c: Adding new entry to the map at index %d", index);
            buck->next = bucket_new(key, value);
            m->size++;
            return;
        }

        buck = buck->next;
    }
}

void *map_get(map *m, char *key)
{
    debug("map.c: Looking for key %s in the map", key);

    int index = get_index(m, key);

    bucket_list *list = m->table + index;

    if (list == NULL) {
        debug("map.c: Couldn't find %s, the map is currently empty.", key);
        return NULL;
    }

    bucket *buck = list->begin;

    if (buck == NULL) {
        debug("map.c: Couldn't find %s, there are no buckets at position %d", key, index);
        return NULL;
    }

    while (buck != NULL) {
        if (strcmp(buck->entry.key, key) == 0) {
            debug("map.c: Found a match for key %s in the map", key);
            return buck->entry.value;
        }
        debug("map.c: Key %s at position %d does not match search key %s", buck->entry.key, index, key);
        buck = buck->next;
    }
    return NULL;
}

void map_del(map *m, char *key)
{
    int index = get_index(m, key);

    bucket_list *list = m->table + index;

    bucket *buck = list->begin;

    if (buck == NULL) {
        return;
    }

    bucket *prev = NULL;
    while (buck != NULL) {
        if (strcmp(buck->entry.key, key) == 0) {
            if (prev != NULL) {
                prev->next = buck->next;
            } else {
                list->begin = buck->next;
            }
            break;
        }
        prev = buck;
        buck = buck->next;
    }

    if (buck != NULL) {
        // TODO: Add a flag to allow freeing the map's keys and values
        // free(buck->entry.key);
        // free(buck->entry.value);
        free(buck);
    }
}

char should_resize(map *m)
{
    return m->size > m->capacity * 4;
}

unsigned int calc_next_resize(map *m)
{
    unsigned prev_size = 1;
    unsigned resize = 1;

    while (resize <= m->size) {
        int aux = resize;
        resize = prev_size + resize;
        prev_size = aux;
    }

    return resize;
}

void resize_map(map *m, unsigned int new_size)
{
    // Create a new map
    map *aux_map = map_new(new_size);

    // Copy the contents of the current one to the new map.
    for (int i = 0; i < m->capacity && aux_map->size < m->size; i++) {
        if (m->table + i == NULL)
            continue;

        bucket *b = m->table[i].begin;

        while (b != NULL) {
            map_set(aux_map, b->entry.key, b->entry.value);
            b = b->next;
        }
    }

    // Destroy old table
    for (int i = 0; i < m->capacity; i++) {
        if (m->table + i != NULL) {
            bucket_list_destroy(m->table[i].begin, 0, 0);
        }
    }
    free(m->table);

    // Reassign to the new table
    m->table = aux_map->table;
    m->capacity = aux_map->capacity;
    free(aux_map);
}

void map_display(map *m)
{
    for (int i = 0; i < m->capacity; i++) {
        printf("%d: ", i);
        bucket_list *list = m->table + i;

        if (list == NULL) {
            continue;
        }

        bucket *buck = list->begin;
        while (buck != NULL) {
            printf(" %s - ", buck->entry.key);
            buck = buck->next;
        }
        printf("\n");
    }
}

unsigned calc_hash(const char *key)
{
    return qhashmurmur3_32(key, strlen(key));
}

unsigned get_index(map *m, char *key)
{
    unsigned int hash = calc_hash(key);
    return hash % m->capacity;
}

void bucket_list_destroy(bucket *b, char dealloc_keys, char dealloc_vals)
{
    if (b == NULL) {
        return;
    }
    bucket_list_destroy(b->next, dealloc_keys, dealloc_vals);

    if (dealloc_keys) {
        free(b->entry.key);
    }
    if (dealloc_vals) {
        free(b->entry.value);
    }
    free(b);
}

void map_destroy(map *m)
{
    map_destroy_dealloc(m, 0, 0);
}

void map_destroy_dealloc(map *m, char dealloc_keys, char dealloc_vals)
{
    for (int i = 0; i < m->capacity; i++) {
        if (m->table + i != NULL) {
            bucket_list_destroy(m->table[i].begin, dealloc_keys, dealloc_vals);
        }
    }
    free(m->table);
    free(m);
}

/**
 * Get 32-bit Murmur3 hash.
 *
 * @param data      source data
 * @param nbytes    size of data
 *
 * @return 32-bit unsigned hash value.
 *
 * @code
 *  unsigned int hashval = qhashmurmur3_32((void*)"hello", 5);
 * @endcode
 *
 * @code
 *  MurmurHash3 was created by Austin Appleby  in 2008. The initial
 *  implementation was published in C++ and placed in the public.
 *    https://sites.google.com/site/murmurhash/
 *  Seungyoung Kim has ported its implementation into C language
 *  in 2012 and published it as a part of qLibc component.
 * @endcode
 */
unsigned int qhashmurmur3_32(const void *data, size_t nbytes)
{
    if (data == NULL || nbytes == 0)
        return 0;

    const unsigned int c1 = 0xcc9e2d51;
    const unsigned int c2 = 0x1b873593;

    const int nblocks = nbytes / 4;
    const unsigned int *blocks = (const unsigned int *)(data);
    const unsigned char *tail = (const unsigned char *)(data + (nblocks * 4));

    unsigned int h = 0;

    int i;
    unsigned int k;
    for (i = 0; i < nblocks; i++) {
        k = blocks[i];

        k *= c1;
        k = (k << 15) | (k >> (32 - 15));
        k *= c2;

        h ^= k;
        h = (h << 13) | (h >> (32 - 13));
        h = (h * 5) + 0xe6546b64;
    }

    k = 0;
    switch (nbytes & 3) {
    case 3:
        k ^= tail[2] << 16;
    case 2:
        k ^= tail[1] << 8;
    case 1:
        k ^= tail[0];
        k *= c1;
        k = (k << 15) | (k >> (32 - 15));
        k *= c2;
        h ^= k;
    };

    h ^= nbytes;

    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}
