#include "eventschema.h"

#include "list.h"

event_schema
{
    list *fields;
}

event_schema *event_schema_new()
{
    event_schema *schema = (event_schema *)malloc(sizeof(event_schema));
    schema->fields = list_new();
    return schema;
}

void event_schema_del(event_schema *schema)
{
    list_destroy(schema->fields);
}

void event_schema_add_field(event_schema *schema, field_config field)
{
}