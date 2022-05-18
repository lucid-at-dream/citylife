#include "line_string.h"
#include "point.h"

#include <stdlib.h>

line_string *line_string_new() {
    line_string *new = (line_string *)calloc(1, sizeof(line_string));
    new->vertex_list = dynarray_new();
    return new;
}

void line_string_destroy(line_string *ls) {
    
    for (int i = 0; i < ls->vertex_list->size; i++) {
        point *p = (point *)dynarray_get(ls->vertex_list, i);
        point_destroy(p);
    }

    dynarray_destroy(ls->vertex_list);
    free(ls);
}
