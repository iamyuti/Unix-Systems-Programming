/**
 * @file http_client.c
 * @brief A basic HTTP client using Stream I/O.
 * * Demonstrates:
 * 1. DNS Resolution (getaddrinfo).
 * 2. TCP Socket Connection.
 * 3. Stream encapsulation (fdopen) to treat Sockets like Files.
 * 4. Sending HTTP GET requests via fprintf.
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

void error_exit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <hostname> <path> [port]\n", argv[0]);
        fprintf(stderr, "Example: %s www.example.com / 80\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *hostname = argv[1];
    const char *path = argv[2];
    const char *port = (argc >= 4) ? argv[3] : "80";

    struct addrinfo hints;
    struct addrinfo *result, *p;

    memset(&hints, 0 , sizeof(hints));
    hints.ai_family = AF_UNSPEC;     // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP

    // Resolve DNS
    int status = getaddrinfo(hostname, port, &hints, &result);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    // Connect to the first valid address
    int sockfd;
    for(p = result; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) < 0) {
            close(sockfd);
            continue;
        }
        break; // Connected successfully
    }

    if (p == NULL) {
        fprintf(stderr, "Failed to connect to %s:%s\n", hostname, port);
        exit(EXIT_FAILURE);
    }
    
    freeaddrinfo(result);

    // Wrap Socket in File Stream
    FILE *sockfile = fdopen(sockfd, "r+"); // "r+" allows reading and writing to the stream
    if (sockfile == NULL) {
        error_exit("fdopen failed");
    }

    // Send HTTP Request
    // Construct the GET request dynamically based on arguments
    if (fprintf(sockfile, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, hostname) < 0) {
        error_exit("fprintf failed");
    }
    
    if (fflush(sockfile) == EOF) {
        error_exit("fflush failed");
    }

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), sockfile) != NULL) {
        fputs(buffer, stdout);
    }

    fclose(sockfile); // Closing stream also closes the socket
    return EXIT_SUCCESS;
}
