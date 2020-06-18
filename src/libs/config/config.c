#include "config.h"
#include <stdlib.h>
#include <errno.h>
#include "logger/logger.h"

map *arg_parse(map *args, int arg_desc_count, arg_t *arg_desc, int argc, char **argv);

map *load_config(int arg_desc_count, arg_t *arg_desc, int argc, char **argv) {
    map *args = map_new(arg_desc_count * 2);

    // TODO: parse/handle built-in arguments (-h|--help, -c|--config-file)

    return arg_parse(args, arg_desc_count, arg_desc, argc, argv);
}


map *parse_config_file(map *args, char *path, int arg_desc_count, arg_t *arg_desc) {
    
    // TODO: parse configuration file

    return args;
}

/**
 * Transforms some given argument list (argc/argv) into a key/value map, provided that a
 * description of the arguments to expect is given.
 */
map *arg_parse(map *args, int arg_desc_count, arg_t *arg_desc, int argc, char **argv) {
    
    map *arg_desc_map = map_new(arg_desc_count * 2);

    for (int i = 0; i < arg_desc_count; i++) {
        map_set(args, arg_desc[i].long_name, NULL);
        map_set(args, arg_desc[i].short_name, 0);
        
        map_set(arg_desc_map, arg_desc[i].long_name, arg_desc + i);
        map_set(arg_desc_map, arg_desc[i].short_name, arg_desc + i);
    }

    int i = 1;
    while (i < argc) {

        arg_t *desc;
        if (argv[i][0] == '-' && argv[i][1] == '-') { // Long name given
            desc = map_get(arg_desc_map, argv[i] + 2);
        } else if (argv[i][0] == '-' && argv[i][1] != '-') { // Short name given
            desc = map_get(arg_desc_map, argv[i] + 1);
        } else {
            error("Unable to parse argument %s", argv[i]);
            printUsage(arg_desc_count, arg_desc);
            exit(EXIT_FAILURE);
        }

        if (desc == NULL) { // Argument not found
            error("Unrecognized command line argument %s", argv[i]);
            printUsage(arg_desc_count, arg_desc);
            exit(EXIT_FAILURE);
        }

        debug("Parsing argument %s\n", desc->long_name);

        // Handle the case of previously parsed and allocated argument values
        if (map_get(args, desc->short_name) && (desc->type == INTEGER || desc->type == FLOAT)) {
            free(map_get(args, desc->long_name));
        }

        // This informs that the argument was set.
        map_set(args, desc->short_name, 1);

        // Here we parse the argument value.
        if (desc->type == INTEGER) {
            char *endptr;
            int *value = (int *)malloc(sizeof(int));
            value[0] = strtol(argv[i+1], &endptr, 10);
            if (*endptr != '\0') {
                free(value);
                error("Unable to parse the following value as an integer '%s'", argv[i + 1]);
                printUsage(arg_desc_count, arg_desc);
                exit(EXIT_FAILURE);
            }
            map_set(args, desc->long_name, value);
            i += 2;
        
        } else if (desc->type == STRING) {
            map_set(args, desc->long_name, argv[i+1]);
            i += 2;
        
        } else if (desc->type == FLOAT) {
            char *endptr;
            double *value = (double *)malloc(sizeof(double));
            value[0] = strtod(argv[i+1], &endptr);
            if (*endptr != '\0') {
                free(value);
                error("Unable to parse the following value as an double '%s'", argv[i + 1]);
                printUsage(arg_desc_count, arg_desc);
                exit(EXIT_FAILURE);
            }
            map_set(args, desc->long_name, value);
            i += 2;

        } else if (desc->type == FLAG) {
            map_set(args, desc->long_name, 1);
            i += 1;
        }
    }

    map_destroy(arg_desc_map);

    return args;
}

/**
 * Prints the usage of the program based on some description of its arguments.
 */
void printUsage(int arg_desc_count, arg_t *arg_desc) {

}

void deallocate_arg_map(int arg_desc_count, arg_t *arg_desc, map *arg_map) {
    for (int i = 0; i < arg_desc_count; i++) {
        if (map_get(arg_map, arg_desc[i].short_name)) {
            if (arg_desc[i].type == INTEGER || arg_desc[i].type == FLOAT) {
                free(map_get(arg_map, arg_desc[i].long_name));
            }
        }
    }
    map_destroy(arg_map);
}
