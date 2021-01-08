#include "config.h"
#include "server.h"
#include "json.h"
#include <stdio.h>

char *register_service(char *args)
{
    return args;
}

char *proxy_user_request(char *args)
{
}

int main(int argc, char **argv)
{
    arg_t arg_desc[] = { { "sp", "service-registry-port", INTEGER, OPTIONAL,
                           "The port where the gateway will be listening for service registrations" },
                         { "rp", "rest-api-port", INTEGER, OPTIONAL, "The port where the gateway will be listening for REST requests" } };

    map *conf = load_config(sizeof(arg_desc) / sizeof(arg_t), arg_desc, argc, argv);

    /*
    socket_server *endpoint_registry_server = server_new(9999);
    server_start(endpoint_registry_server, register_service);

    socket_server *user_requests_server = server_new(80);
    server_start(user_requests_server, proxy_user_request);
    */

    return 0;
}
