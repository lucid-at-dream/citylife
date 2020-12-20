#pragma once

#include "../hashmap/map.h"

/**
 * The type of some argument.
 */
typedef enum _arg_type_t {
        INTEGER,
        FLOAT,
        FLAG,
        STRING,
} arg_type_t;

typedef enum _arg_mandatory_t { OPTIONAL = 0, MANDATORY = 1 } arg_mandatory_t;

/**
 * Description of an argument.
 */
typedef struct _arg_t {
        char *short_name;
        char *long_name;
        arg_type_t type;
        arg_mandatory_t is_mandatory;
        char *doc_string;
} arg_t;

/**
 * Transforms some given argument list (argc/argv) into a key/value map, provided that a
 * description of the arguments to expect is given.
 */
map *load_config(int arg_desc_count, arg_t *arg_desc, int argc, char **argv);

/**
 * Prints the usage of the program based on some description of its arguments.
 */
void print_usage(int arg_desc_count, arg_t *arg_desc);

void deallocate_arg_map(int arg_desc_count, arg_t *arg_desc, map *arg_map);
