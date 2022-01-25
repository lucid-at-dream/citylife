#include <varchar.h>

#include <stdlib.h>
#include <string.h>

varchar *varchar_new(char *data) {
    unsigned data_length = strnlen(data, MAX_VARCHAR_SIZE);

    varchar *new = (varchar *)malloc(sizeof(varchar));
    new->data = (char *)malloc((data_length + 1) * sizeof(char));

    strncpy(new->data, data, data_length);
    new->length = data_length;

    return new;
}

void varchar_del(varchar *var) {
    free(var->data);
}
