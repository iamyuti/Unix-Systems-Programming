CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -Wextra -D_POSIX_C_SOURCE=200809L -g

.PHONY: all clean

all: http_client http_server

http_client: http_client.c
	$(CC) $(CFLAGS) -o http_client http_client.c

http_server: http_server.c
	$(CC) $(CFLAGS) -o http_server http_server.c

clean:
	rm -f http_client http_server
