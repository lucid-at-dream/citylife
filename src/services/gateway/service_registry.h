#pragma once

#include "map.h"

map *registry_new();

void registry_destroy(map *registry);

char *get_forwarding_address(map *registry);
