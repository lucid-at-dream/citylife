#include "json.h"

#include "map.h"
#include "jsmn.h"

void *parse_json_element(jsmntok_t *tokens, int *curr_token, char *json);

void *parse_json(char *json)
{
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

void *parse_json_element(jsmntok_t *tokens, int *curr_token, char *json) {

    jsmntok_t tok = tokens[*curr_token];

    switch (tok.type) {
        case JSMN_OBJECT:
        {
            *curr_token += 1; // Move from obj to key

            map *object = map_new(tok.size * 2);
            for (int count = 0; count < tok.size; count++) {

                // Extract the key (a string)
                int key_len = tokens[*curr_token].end - tokens[*curr_token].start;
                char *key = (char *)calloc(key_len + 1, sizeof(char));
                strncpy(key, json + tokens[*curr_token].start, key_len);
                
                *curr_token += 1; // Move from key to value

                // Extract the value
                void *value = parse_json_element(tokens, curr_token, json);

                // Store the result
                map_set(object, key, value);
            }

            return object;
        }

        case JSMN_ARRAY:
            return NULL;

        case JSMN_STRING:
        case JSMN_PRIMITIVE:
        {
            int key_len = tok.end - tok.start;
            char *key = (char *)calloc(key_len + 1, sizeof(char));
            strncpy(key, json + tok.start, key_len);
            *curr_token += 1;
            return key;
        }

        case JSMN_UNDEFINED:
        {
            // TODO: Throw an error
            return NULL;
        }

        default:
        {
            return NULL;
        }
    }
}
