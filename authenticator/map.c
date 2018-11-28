#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "map.h"

map *map_new(int capacity) {
  map *m = (map *)malloc(sizeof(map));
  m->capacity = capacity;
  m->size = 0;
  m->table = (bucket_list *)calloc(capacity, sizeof(bucket_list));
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

  map_entry entry = {key, value};

  bucket *new_bucket = (bucket *)malloc(sizeof(bucket));
  new_bucket->entry = entry;
  new_bucket->next =  NULL;

  int index = get_index(m, key);
  bucket_list *list = m->table + index;
  bucket *buck = list->begin;
  
  if (buck == NULL) {
    list->size = 1;
    list->begin = new_bucket;
    return;
  }
  
  while(buck != NULL) {
    if (buck->next == NULL) {
      buck->next = new_bucket;
      return;
    }
    buck = buck->next;
  }
}

char *map_get(map *m, char *key) {

  int index = get_index(m, key);

  bucket_list *list =  m->table + index;

  bucket *buck = list->begin;

  if (buck == NULL) {
    return NULL;
  }
  
  while(buck != NULL) {
    if (strcmp(buck->entry.key, key) == 0) {
      return buck->entry.value;
    }
    printf("compare: %s to %s\n", buck->entry.key, key);
    buck = buck->next;
  }
  return NULL;
}

void map_display(map *m) {
  for (int i = 0; i < m->capacity; i++) {
    printf("%d: ", i);
    bucket_list *list = m->table + i;
    if (list == NULL) {
      printf("table is null");
    }
    bucket *buck = list->begin;
    while (buck != NULL) {
      printf(" %s - ", buck->entry.key);
      buck = buck->next;
    } 
    printf("\n");
  } 
}

void map_destroy(map *m) {
  
}

