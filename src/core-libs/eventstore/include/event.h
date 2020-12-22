#pragma once

#include "event_structs.h"

#include <stdlib.h>

event *event_new();

void event_del(event *e);

void event_set_field(event *e, char *field_name, void *value, size_t value_size);
