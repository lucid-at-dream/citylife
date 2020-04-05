#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "server.h"
#include "logger/logger.h"
#include "workerpool/worker_pool.h"

socket_server *server_new(unsigned short port) {
    info("server.c: Creating a new server bound to port %d", port);
    socket_server *new_server = (socket_server *)calloc(1, sizeof(socket_server));
    new_server->server_port = port;
    new_server->protoent = getprotobyname("tcp");
    return new_server;
}

void server_stop(socket_server *server) {
    info("server.c: Stopping the server");
    server->stop = 1;
}

void server_start(socket_server *server, char *(*handle_request)(char *)) {

    server->server_address.sin_family = AF_INET;
    server->server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server->server_address.sin_port = htons(server->server_port);

    server->server_sock_file_descriptor = socket(AF_INET, SOCK_STREAM, server->protoent->p_proto);
    if (server->server_sock_file_descriptor == -1) {
        perror("Could not create the server socket.");
        exit(EXIT_FAILURE);
    }
    
    if (bind(server->server_sock_file_descriptor, (struct sockaddr*)&server->server_address, sizeof(server->server_address)) == -1) {
        perror("Could not bind the server socket to the given port.");
        exit(EXIT_FAILURE);
    }

    if (listen(server->server_sock_file_descriptor, 1000) == -1) {
        perror("Unable to start listening for requests.");
        exit(EXIT_FAILURE);
    }
    
    info("server.c: Listening on port %d", server->server_port);

    struct sockaddr_in client_address;
    while (!server->stop) {

        debug("server.c: server stop flag is set to %d", server->stop);

        socklen_t client_len = sizeof(client_address);
        
        // Accept a new connection
        server->client_sock_file_descriptor = accept(
            server->server_sock_file_descriptor,
            (struct sockaddr*)&client_address,
            &client_len
        );

        // Read the request
        int read_bytes = read(server->client_sock_file_descriptor, server->buffer, BUFSIZE);
        if (read_bytes <= 1) {
            debug("server.c: Server read unblocked but no data was read");
            continue;
        }

        info("server.c: Request with %d Bytes received", read_bytes);

        // Handle the request
        char *response = handle_request(server->buffer);
        
        // Write the response
        write(server->client_sock_file_descriptor, response, strlen(response));
        
        // Close the file descriptor and deallocate resources
        close(server->client_sock_file_descriptor);
        free(response);
    }

    info("server.c: Server exiting from server loop");
    close(server->server_sock_file_descriptor);
}
