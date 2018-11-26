#include <stdlib.h>

#include "map.h"

map *map_new(int capacity) {
  map *m = (map *)malloc(sizeof(map));
  m->capacity = capacity;
  m->size = 0;
  m->keys = (char **)malloc(capacity * sizeof(char *));
  m->values = (char **)malloc(capacity * sizeof(char *));
  return m;
}

void map_add(map *m, char *key, char *value) {
  m->keys[0] = key;
  m->values[0] = value;
}

char *map_get(map *m, char *key) {
  return m->values[0];
}

void map_destroy(map *m) {
  free(m->keys);
  free(m->values);
}

