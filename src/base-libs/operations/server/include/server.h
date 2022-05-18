#pragma once

#define BUFSIZE 1024

#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct _socket_server
{
    unsigned short server_port;
    int server_sock_file_descriptor;
    int client_sock_file_descriptor;
    struct protoent *protoent;
    struct sockaddr_in server_address;
    volatile char stop;
    char buffer[BUFSIZE];
} socket_server;

socket_server *server_new(unsigned short port);

void server_stop(socket_server *server);

void server_start(socket_server *server, char *(*handle_request)(char *));
