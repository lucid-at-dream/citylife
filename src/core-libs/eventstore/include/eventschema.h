#pragma once

#include "datatypes.h"
#include "event_structs.h"

/**
 * Allocates an event schema based on the specified schema.
 */
event_schema *event_schema_new();

/**
 * Deallocates an event schema.
 */
void event_schema_del(event_schema *);

/**
 * Adds a field to the schema.
 */
void event_schema_add_field(event_schema *, field_config);
