#include "eventschema.h"

#include "list.h"

#include <stdlib.h>

struct _event_schema
{
    list *fields;
};

event_schema *event_schema_new()
{
    event_schema *schema = (event_schema *)malloc(sizeof(event_schema));
    schema->fields = list_new();
    return schema;
}

void free_schema_field(void **field) {
    free(*field);
}

void event_schema_del(event_schema *schema)
{
    list_foreach(schema->fields, free_schema_field);
    list_destroy(schema->fields);
    free(schema);
}

void event_schema_add_field(event_schema *schema, field_config field)
{
    field_config *f = (field_config *)malloc(sizeof(field_config));
    f->name = field.name;
    f->type = field.type;
    list_append(schema->fields, f);
}