#include "config.h"
#include <stdlib.h>
#include "logger/logger.h"

/**
 * Transforms some given argument list (argc/argv) into a key/value map, provided that a
 * description of the arguments to expect is given.
 */
map *arg_parse(int arg_desc_count, arg_t *arg_desc, int argc, char **argv) {

    map *args = map_new(arg_desc_count * 2);
    
    map *arg_desc_map = map_new(arg_desc_count * 2);

    for (int i = 0; i < arg_desc_count; i++) {
        map_set(args, arg_desc[i].long_name, NULL);
        map_set(args, arg_desc[i].short_name, NULL);
        
        map_set(arg_desc_map, arg_desc[i].long_name, arg_desc + i);
        map_set(arg_desc_map, arg_desc[i].short_name, arg_desc + i);
    }

    for (int i = 1; i < argc; i++) {

        if (argv[i][0] == '-' && argv[i][1] == '-') { // Long name given

            debug("Parsing argument %s\n", argv[i] + 2);

            char *key = argv[i] + 2;
            arg_t *desc = map_get(arg_desc_map, key);

            if (desc->type == INTEGER) {
                map_set(args, argv[i] + 2, atoi(argv[i+1]));
            
            } else if (desc->type == STRING) {
                map_set(args, argv[i] + 2, argv[i+1]);
            }
                

            i += 1;

        } else if (argv[i][0] == '-' && argv[i][1] != '-') { // Short name given
            // Do nothing for now, TDD

        } else {
            // Dunno, something's wrogn with the given argumets
        }
    }

    map_destroy(arg_desc_map);

    return args;
}

/**
 * Prints the usage of the program based on some description of its arguments.
 */
void printUsage(arg_t *args) {

}

