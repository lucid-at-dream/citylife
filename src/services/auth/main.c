#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "service.h"

// Default arguments
int port = 9999;

// Declared functions
void printHelp(FILE *stream);

int main(int argc, char **argv) {

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0) {
            if (i + 1 >= argc) {
                printHelp(stderr);
                exit(-1);
            }
            port = atoi(argv[++i]);
        }
        if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printHelp(stdout);
            exit(0);
        }
    }

    service_run(port);

    return 0;
}

void printHelp(FILE *stream) {
    fprintf(stream, "=== Usage -- Authentication microservice from Citylife ===\n");
    fprintf(stream, "auth.exe [-p|--port port] [-h|--help]\n");
    fprintf(stream, "\n");
    fprintf(stream, "-h|--help   Print this help message and exit\n");
    fprintf(stream, "-p|--port   The port in which to listen for requests\n");
}

