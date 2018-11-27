
typedef struct _map {
  int capacity;
  int size;
  char **keys;
  char **values;  
} map;

map *map_new(int capacity);

void map_add(map *m, char *key, char *value);
char *map_get(map *m, char *key);
void map_destroy(map *m);
