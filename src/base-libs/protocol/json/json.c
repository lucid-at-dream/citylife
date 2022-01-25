#include "json.h"

#include "map.h"
#include "jsmn.h"

json_object *parse_json_element(jsmntok_t *tokens, int *curr_token, char *json);

json_object *parse_json(char *json) {
    // Initialize parser
    jsmn_parser parser;
    jsmn_init(&parser);

    // Tokenize the json
    jsmntok_t tokens[256];
    jsmn_parse(&parser, json, strlen(json), tokens, 256);

    // Parse the tokens
    int parsed_tokens = 0;
    return parse_json_element(tokens, &parsed_tokens, json);
}

json_object *parse_json_element(jsmntok_t *tokens, int *curr_token, char *json) {
    jsmntok_t tok = tokens[*curr_token];

    switch (tok.type) {
    case JSMN_OBJECT: {
        *curr_token += 1; // Move from obj to key

        map *object = map_new(tok.size * 2);
        for (int count = 0; count < tok.size; count++) {
            // Extract the key (a string)
            int key_len = tokens[*curr_token].end - tokens[*curr_token].start;
            char *key = (char *)calloc(key_len + 1, sizeof(char));
            strncpy(key, json + tokens[*curr_token].start, key_len);

            *curr_token += 1; // Move from key to value

            // Extract the value
            json_object *value = parse_json_element(tokens, curr_token, json);

            // Store the result
            map_set(object, key, value);
        }

        json_object_content content;
        content.object = object;

        json_object *result = (json_object *)calloc(1, sizeof(json_object));
        result->type = JSON_OBJECT;
        result->content = content;

        return result;
    }

    case JSMN_ARRAY: {
        json_object_content content;
        content.array = NULL;

        json_object *result = (json_object *)calloc(1, sizeof(json_object));
        result->type = JSON_ARRAY;
        result->content = content;
        return result;
    }

    case JSMN_STRING:
    case JSMN_PRIMITIVE: {
        int data_len = tok.end - tok.start;
        char *data = (char *)calloc(data_len + 1, sizeof(char));
        strncpy(data, json + tok.start, data_len);
        *curr_token += 1;

        json_object_content content;
        content.data = data;

        json_object *result = (json_object *)calloc(1, sizeof(json_object));
        result->type = JSON_PRIMITIVE;
        result->content = content;

        return result;
    }

    default: {
        json_object *result = (json_object *)calloc(1, sizeof(json_object));
        result->type = JSON_ERROR;
        return result;
    }
    }
}

void free_json_object_values(map *m, char *key, void *args) {
    json_object *obj = (json_object *)map_get(m, key);
    json_dealloc(obj);
}

void json_dealloc(json_object *json) {
    switch (json->type) {
    case JSON_PRIMITIVE: {
        free(json->content.data);
        free(json);
        break;
    }

    case JSON_OBJECT: {
        map_iter_keys(json->content.object, free_json_object_values, NULL);
        map_destroy_dealloc(json->content.object, 1, 0);
        free(json);
        break;
    }

    default:
        break;
    }
}