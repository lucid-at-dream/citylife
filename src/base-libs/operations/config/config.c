#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdarg.h>

#include "config.h"
#include "logger.h"

/* Built-in arguments */
arg_t _config_help_arg_desc = { "h", "help", FLAG, OPTIONAL, "Display this help message" };
arg_t _config_cfg_file_arg_desc = { "c", "config", STRING, OPTIONAL, "Load config from the configuration file at the given path" };

// Helpers for load config
map *parse_command_line(map *args, int arg_desc_count, arg_t *arg_desc, int argc, char **argv);
map *parse_config_file(map *args, char *path, int arg_desc_count, arg_t *arg_desc);
char parse_argument_value_pair(map *args, arg_t *desc, char *value);

typedef struct _parse_status {
    int arg_desc_count;
    arg_t *arg_desc;
    map *args;
    char *msg;
} parse_status;

__attribute__((noreturn)) void elegant_exit(int arg_desc_count, arg_t *arg_desc, map *args, char *msg, ...) {
    va_list argptr;
    va_start(argptr, msg);

    error(msg, argptr);

    va_end(argptr);

    print_usage(arg_desc_count, arg_desc);

    deallocate_arg_map(arg_desc_count, arg_desc, args);
    pthread_exit(EXIT_FAILURE);
}

map *load_config(int arg_desc_count, arg_t *arg_desc, int argc, char **argv) {
    map *args = map_new(arg_desc_count * 2 + 2);

    // Initialize the arg maps at 0
    for (int i = 0; i < arg_desc_count; i++) {
        map_set(args, arg_desc[i].long_name, NULL);
        map_set(args, arg_desc[i].short_name, 0);
    }

    // Check if a configuration file has been given.
    for (int i = 0; i < argc; i++) {
        if (strlen(argv[i]) < 2) {
            elegant_exit(arg_desc_count, arg_desc, args, "Bad argument: %s\n", argv[i]);
        }

        const char *desc;
        if (argv[i][0] == '-' && argv[i][1] == '-') { // Long name given
            desc = argv[i] + 2;
        } else if (argv[i][0] == '-' && argv[i][1] != '-') { // Short name given
            desc = argv[i] + 1;
        } else {
            continue;
        }

        if (strcmp(desc, _config_cfg_file_arg_desc.short_name) == 0 || strcmp(desc, _config_cfg_file_arg_desc.long_name) == 0) {
            if (argc <= i + 1) {
                elegant_exit(arg_desc_count, arg_desc, args, "No path given for the configuration file.");
            }
            parse_config_file(args, argv[i + 1], arg_desc_count, arg_desc);
        }

        if (strcmp(desc, _config_help_arg_desc.short_name) == 0 || strcmp(desc, _config_help_arg_desc.long_name) == 0) {
            print_usage(arg_desc_count, arg_desc);
            deallocate_arg_map(arg_desc_count, arg_desc, args);
            pthread_exit(EXIT_SUCCESS);
        }
    }

    return parse_command_line(args, arg_desc_count, arg_desc, argc, argv);
}

char is_blank_line(const char *line) {
    int i = 0;
    while (line[i] == '\t' || line[i] == ' ')
        i++;
    if (line[i] != '\n')
        return 0;
    return 1;
}

int parse_config_file_line(map *args, char *line, int arg_desc_count, arg_t *arg_desc) {
    int first_whitespace_index = 0;
    while (line[first_whitespace_index] != ' ' && first_whitespace_index < 2048) {
        first_whitespace_index++;
    }

    char *value = line + first_whitespace_index + 1;
    line[first_whitespace_index] = '\0';

    char arg_parse_status = -10; // set to -10 so that if arg not found, we know.
    for (int i = 0; i < arg_desc_count; i++) {
        if (strcmp(line, arg_desc[i].short_name) == 0 || strcmp(line, arg_desc[i].long_name) == 0) {
            arg_parse_status = parse_argument_value_pair(args, &(arg_desc[i]), value);
            break;
        }
    }

    return arg_parse_status; 
}

map *parse_config_file(map *args, char *path, int arg_desc_count, arg_t *arg_desc) {
    info("Loading configuration from file %s\n", path);

    FILE *f = fopen(path, "r");

    size_t size = sizeof(char) * 2048;
    char *line = (char *)malloc(size);

    long int nbytes;
    while ((nbytes = getline(&line, &size, f)) != -1) {
        if (line[0] != '#' && !is_blank_line(line)) {
            if (line[nbytes - 1] == '\n') {
                line[nbytes - 1] = '\0';
            }
            
            
            int arg_parse_status = parse_config_file_line(args, line, arg_desc_count, arg_desc);

            if (arg_parse_status == -10) {
                free(line);
                fclose(f);
                elegant_exit(arg_desc_count, arg_desc, args, "Configuration property '%s' is not known.\n", line);
            } else if (arg_parse_status < 0) {
                free(line);
                fclose(f);
                elegant_exit(arg_desc_count, arg_desc, args, "Error parsing config file line: %s", line);
            }
        }
    }

    free(line);
    fclose(f);

    return args;
}

/**
 * Transforms some given argument list (argc/argv) into a key/value map, provided that a
 * description of the arguments to expect is given.
 */
map *parse_command_line(map *args, int arg_desc_count, arg_t *arg_desc, int argc, char **argv) {
    map *arg_desc_map = map_new(arg_desc_count + 5);

    for (int i = 0; i < arg_desc_count; i++) {
        map_set(arg_desc_map, arg_desc[i].long_name, arg_desc + i);
        map_set(arg_desc_map, arg_desc[i].short_name, arg_desc + i);
    }

    int i = 1;
    while (i < argc) {
        const char *arg_name = NULL;
        arg_t *desc = NULL;
        if (argv[i][0] == '-' && argv[i][1] == '-') { // Long name given
            arg_name = argv[i] + 2;
            desc = map_get(arg_desc_map, argv[i] + 2);

        } else if (argv[i][0] == '-' && argv[i][1] != '-') { // Short name given
            arg_name = argv[i] + 1;
            desc = map_get(arg_desc_map, argv[i] + 1);

        } else {
            map_destroy(arg_desc_map);
            elegant_exit(arg_desc_count, arg_desc, args, "Unable to parse argument %s", argv[i]);
        }

        // Parse built-in config file argument
        if (strcmp(arg_name, _config_cfg_file_arg_desc.short_name) == 0 || strcmp(arg_name, _config_cfg_file_arg_desc.long_name) == 0) {
            map_set(args, _config_cfg_file_arg_desc.short_name, 1);

            int size = strlen(argv[i + 1]);
            char *parsed_value = (char *)calloc(size + 1, sizeof(char));
            snprintf(parsed_value, size + 1, "%s", argv[i + 1]);

            map_set(args, _config_cfg_file_arg_desc.long_name, parsed_value);
            i += 2;

            continue;
        }

        if (desc == NULL) { // Argument not found
            map_destroy(arg_desc_map);
            elegant_exit(arg_desc_count, arg_desc, args, "Unrecognized command line argument %s", argv[i]);
        }

        // Parse the argument's value.
        char config_parse_status;
        if (desc->type != FLAG) {
            config_parse_status = parse_argument_value_pair(args, desc, argv[i + 1]);
            i += 2;
        } else {
            config_parse_status = parse_argument_value_pair(args, desc, NULL);
            i += 1;
        }

        // Check if config parse resulted in error. If so, exit elegantly.
        if (config_parse_status < 0) {
            map_destroy(arg_desc_map);
            elegant_exit(arg_desc_count, arg_desc, args, "Failed to parse argument %s\n", desc->long_name);
        }
    }

    map_destroy(arg_desc_map);

    return args;
}

char parse_argument_value_pair(map *args, arg_t *desc, char *value) {
    map_set(args, desc->short_name, 1);

    // Handle the case of previously parsed and allocated argument values
    // Deallocate previously allocated items in case of override.
    if (map_get(args, desc->short_name) && desc->type != FLAG) {
        free(map_get(args, desc->long_name));
    }

    // Parse the actual arg, value pair.
    if (desc->type == INTEGER) {
        char *endptr;
        long int *parsed_value = (long int *)malloc(sizeof(long int));
        *parsed_value = strtol(value, &endptr, 10);
        if (*endptr != '\0') {
            free(parsed_value);
            error("Unable to parse the following value as an integer '%s'", value);
            return -1;
        }
        map_set(args, desc->long_name, parsed_value);

    } else if (desc->type == STRING) {
        unsigned long int size = strlen(value);
        char *parsed_value = (char *)calloc(size + 1, sizeof(char));
        snprintf(parsed_value, size + 1, "%s", value);
        map_set(args, desc->long_name, parsed_value);

    } else if (desc->type == FLOAT) {
        char *endptr;
        double *parsed_value = (double *)malloc(sizeof(double));
        *parsed_value = strtod(value, &endptr);
        if (*endptr != '\0') {
            free(parsed_value);
            error("Unable to parse the following value as a double '%s'", value);
            return -1;
        }
        map_set(args, desc->long_name, parsed_value);

    } else if (desc->type == FLAG) {
        map_set(args, desc->long_name, 1);
    }

    return 0;
}

void print_argument(arg_t *arg_desc, int final_size) {
    unsigned long int size = strlen(arg_desc->short_name) + strlen(arg_desc->long_name) + 6;

    int padding = final_size - size + 2;

    printf("  -%s | --%s ", arg_desc->short_name, arg_desc->long_name);
    for (int i = 0; i < padding; i++) {
        printf(" ");
    }
    if (arg_desc->doc_string != NULL) {
        printf("%s", arg_desc->doc_string);
    }
    printf("\n");
}

/**
 * Prints the usage of the program based on some description of its arguments.
 */
void print_usage(int arg_desc_count, arg_t *arg_desc) {
    printf("Usage:\n");
    printf("\n");

    int max_size = 14;
    for (int i = 0; i < arg_desc_count; i++) {
        unsigned long int size = strlen(arg_desc[i].short_name) + strlen(arg_desc[i].long_name) + 6;
        if (size > max_size) {
            max_size = size;
        }
    }

    printf("Mandatory Arguments:\n");
    for (int i = 0; i < arg_desc_count; i++) {
        if (arg_desc[i].is_mandatory) {
            print_argument(arg_desc + i, max_size);
        }
    }
    printf("\n");

    printf("Options:\n");
    print_argument(&_config_help_arg_desc, max_size);
    print_argument(&_config_cfg_file_arg_desc, max_size);
    for (int i = 0; i < arg_desc_count; i++) {
        if (!arg_desc[i].is_mandatory) {
            print_argument(arg_desc + i, max_size);
        }
    }
    printf("\n");
}

void deallocate_arg_map(int arg_desc_count, arg_t *arg_desc, map *arg_map) {
    for (int i = 0; i < arg_desc_count; i++) {
        if (map_get(arg_map, arg_desc[i].short_name) && arg_desc[i].type != FLAG) {
            free(map_get(arg_map, arg_desc[i].long_name));
        }
    }

    if (map_get(arg_map, _config_cfg_file_arg_desc.short_name)) {
        free(map_get(arg_map, _config_cfg_file_arg_desc.long_name));
    }

    map_destroy(arg_map);
}
