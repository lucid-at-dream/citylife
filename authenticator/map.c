#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "map.h"

map *map_new(int capacity) {
  map *m = (map *)malloc(sizeof(map));
  m->capacity = capacity;
  m->size = 0;
  m->keys = (char **)malloc(capacity * sizeof(char *));
  m->values = (char **)malloc(capacity * sizeof(char *));
  return m;
}

int calc_hash(char *key) {
  int hash = 0;
  for(int i = 0; i < strlen(key) && i < 4; i++) {
    hash += key[i] << 8 * i;
  }
  return hash;
}

int get_index(map *m, char *key) {
  int hash = calc_hash(key);
  return hash % m->capacity;
}

void map_add(map *m, char *key, char *value) {
  int index = get_index(m, key);
  m->keys[index] = key;
  m->values[index] = value;
  m->size += 1;
}

char *map_get(map *m, char *key) {
  int index = get_index(m, key);
  return m->values[index];
}

void map_destroy(map *m) {
  free(m);
  free(m->keys);
  free(m->values);
}

