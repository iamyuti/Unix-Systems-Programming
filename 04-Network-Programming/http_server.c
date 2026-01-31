/**
 * @file http_server.c
 * @brief A single-threaded HTTP Server.
 * * Demonstrates:
 * 1. Server Socket Setup (Bind, Listen, Accept).
 * 2. Stream I/O for request parsing (fdopen).
 * 3. Basic HTTP/1.1 Protocol handling (200 OK, 404 Not Found).
 * 4. File serving using standard I/O.
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BACKLOG 10  // How many pending connections queue will hold
#define BUFFER_SIZE 4096

// Send a simple HTTP response
void send_response(FILE *client_file, int status, const char *status_msg, const char *content_type, const char *body) {
    fprintf(client_file, "HTTP/1.1 %d %s\r\n", status, status_msg);
    fprintf(client_file, "Server: SimpleCServer/1.0\r\n");
    fprintf(client_file, "Content-Type: %s\r\n", content_type);
    fprintf(client_file, "Content-Length: %lu\r\n", strlen(body));
    fprintf(client_file, "Connection: close\r\n");
    fprintf(client_file, "\r\n"); // End of headers
    fprintf(client_file, "%s", body);
    fflush(client_file);
}

// Serve a static file
void serve_file(FILE *client_file, const char *filepath) {
    FILE *f = fopen(filepath, "rb");
    if (!f) {
        send_response(client_file, 404, "Not Found", "text/plain", "Error 404: File not found.");
        return;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    // Send Headers
    fprintf(client_file, "HTTP/1.1 200 OK\r\n");
    fprintf(client_file, "Server: SimpleCServer/1.0\r\n");
    fprintf(client_file, "Content-Length: %ld\r\n", fsize);
    fprintf(client_file, "Connection: close\r\n");
    fprintf(client_file, "\r\n");

    // Send File Content
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        fwrite(buffer, 1, bytes_read, client_file);
    }
    
    fflush(client_file);
    fclose(f);
}

void handle_client(int client_fd) {
    FILE *client_file = fdopen(client_fd, "r+");
    if (!client_file) {
        close(client_fd);
        return;
    }

    char request_line[1024];
    if (fgets(request_line, sizeof(request_line), client_file) == NULL) {
        fclose(client_file);
        return;
    }

    char method[16], path[256], protocol[16];
    if (sscanf(request_line, "%15s %255s %15s", method, path, protocol) != 3) {
        send_response(client_file, 400, "Bad Request", "text/plain", "Malformed Request");
        fclose(client_file);
        return;
    }

    printf("[Request] %s %s\n", method, path);

    if (strcmp(method, "GET") != 0) {
        send_response(client_file, 501, "Not Implemented", "text/plain", "Only GET is supported");
        fclose(client_file);
        return;
    }

    if (strstr(path, "..")) {
        send_response(client_file, 403, "Forbidden", "text/plain", "Access Denied");
        fclose(client_file);
        return;
    }

    char filepath[512];
    if (strcmp(path, "/") == 0) {
        snprintf(filepath, sizeof(filepath), "index.html");
    } else {
        // Remove leading slash for local file path
        snprintf(filepath, sizeof(filepath), ".%s", path);
    }

    serve_file(client_file, filepath);
    
    fclose(client_file);
}

int main(int argc, char *argv[]) {
    const char *port = (argc >= 2) ? argv[1] : "8080";

    struct addrinfo hints, *res, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;     // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE;     // For wildcard IP address

    int status;
    if ((status = getaddrinfo(NULL, port, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return EXIT_FAILURE;
    }

    int sockfd;
    int yes = 1;

    // Looping through results and bind to the first we can
    for (p = res; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }

        // Allow creating socket even if port is in TIME_WAIT
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            continue;
        }

        break;
    }

    freeaddrinfo(res);

    if (p == NULL) {
        fprintf(stderr, "Server: failed to bind\n");
        return EXIT_FAILURE;
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        return EXIT_FAILURE;
    }

    printf("Server listening on port %s...\n", port);

    while (1) {
        struct sockaddr_storage their_addr;
        socklen_t sin_size = sizeof(their_addr);
        int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        // Iterative server (handles one at a time)
        // For concurrency, you would fork() here
        handle_client(new_fd);
    }

    return EXIT_SUCCESS;
}
