#include <glib-object.h>
#include <json-glib/json-glib.h>

#include <stdio.h>
#include <stdlib.h>

#include "requests_resolver.h"
#include "auth_verbs.h"

/*Private functions*/
auth_request *parse_request(char *request);
char *get_value_from_json_node(JsonObject *json_dict, char *key);

/*Global variables*/
char *(*callbacks[AUTH_VERBS_COUNT])(auth_request *);

/*Implementation*/

void requests_set_callback(auth_verb action, char *(*callback)(auth_request *)) {
    callbacks[action] = callback;
}

void free_request(auth_request *r) {
  if(r->username) free(r->username);
  if(r->password) free(r->password);
  if(r->session_token) free(r->session_token);
  if(r) free(r);
}

char *requests_resolve(char *request) {
    auth_request *r = parse_request(request);

    char *response;

    if (r != NULL) 
    {
        response = callbacks[r->action](r);
        free_request(r);
    } else {
        response = "{msg: \"Bad request, you scumbag!\"}";
    }

    return response;
}

auth_request *parse_request(char *request) {
    JsonParser *parser = json_parser_new();

    // Parse the request json
    GError *error = NULL;
    json_parser_load_from_data (parser, request, strlen(request), &error);
    if (error != NULL) {
        printf("Unable to parse request: %s\n", error->message);
        g_error_free(error);
        g_object_unref(parser);
        return NULL;
    }

    // Translate the json tree into an auth_request struct
    JsonObject *json_dict = json_node_get_object(json_parser_get_root(parser));
    auth_request *r = (auth_request *)calloc(1, sizeof(auth_request));
    
    // Parse action
    char *action = json_node_get_string(json_object_get_member(json_dict, "action"));
    
    r->action = verb_translate_from_string(action);
    if (r->action == AUTH_INVALID_REQUEST) {
        free(r);
        g_object_unref(parser);
        return NULL;
    }

    r->username = get_value_from_json_node(json_dict, "user");
    r->password = get_value_from_json_node(json_dict, "pass");
    r->session_token = get_value_from_json_node(json_dict, "sess");

    // Free allocated resources
    g_object_unref(parser);

    // Return the built auth_request struct
    return r;
}

char *get_value_from_json_node(JsonObject *json_dict, char *key) {
    
    char *value = NULL;
    JsonNode *node = json_object_get_member(json_dict, key);

    if(JSON_NODE_TYPE(node) != JSON_NODE_NULL) {
        char *tmp_value = json_node_get_string(node);
        int str_size = strlen(tmp_value);
        value = (char *)calloc(str_size + 1, sizeof(char));
        strncpy(value, tmp_value, str_size);
    }

    return value;
}


/*
auth_request *parse_request(char *request) {
    
    JsonParser *parser = json_parser_new();

    // Parse the request json
    GError *error = NULL;
    json_parser_load_from_data (parser, request, strlen(request), &error);
    if (error != NULL) {
      printf("Unable to parse request: %s\n", error->message);
      g_error_free(error);
      g_object_unref(parser);
      return NULL;
    }

    // Translate the json tree into an auth_request struct
    JsonObject *json_dict = json_node_get_object(json_parser_get_root(parser));
    auth_request *r = (auth_request *)calloc(1, sizeof(auth_request));
    
    // Parse action
    char *action = json_node_get_string(json_object_get_member(json_dict, "action"));
    r->action = verb_translate_from_string(action);
    if (r->action == AUTH_INVALID_REQUEST) {
        g_object_unref(parser);
        return NULL;
    }

    // Parse session_token
    char *action = json_node_get_string(json_object_get_member(json_dict, "action"));
    r->action = verb_translate_from_string(action);
    if (r->action == AUTH_INVALID_REQUEST) {
        g_object_unref(parser);
        return NULL;
    }

    // Parse password
    char *action = json_node_get_string(json_object_get_member(json_dict, "action"));
    r->action = verb_translate_from_string(action);
    if (r->action == AUTH_INVALID_REQUEST) {
        g_object_unref(parser);
        return NULL;
    }

    // Parse user
    char *action = json_node_get_string(json_object_get_member(json_dict, "action"));
    r->action = verb_translate_from_string(action);
    if (r->action == AUTH_INVALID_REQUEST) {
        g_object_unref(parser);
        return NULL;
    }

    // Free allocated resources
    g_object_unref (parser);

    // Return the built auth_request struct
    return r;
}

void free_request(auth_request *r) {
    if(r == NULL) {
        return;
    }
    
    if(r->session_token != NULL) free(r->session_token);
    if(r->username != NULL) free(r->username);
    if(r->password != NULL) free(r->password);

    free(r);
}

int main (int argc, char *argv[])
{
  JsonParser *parser;
  JsonNode *root;
  GError *error;


  error = NULL;
  json_parser_load_from_file (parser, argv[1], &error);
  if (error)
    {
      g_print ("Unable to parse `%s': %s\n", argv[1], error->message);
      g_error_free (error);
      g_object_unref (parser);
      return EXIT_FAILURE;
    }


  return EXIT_SUCCESS;
}
*/