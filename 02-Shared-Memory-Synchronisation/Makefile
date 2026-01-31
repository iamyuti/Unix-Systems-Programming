CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -Wextra -D_POSIX_C_SOURCE=200809L -g
LDFLAGS = -lrt -pthread

.PHONY: all clean

all: supervisor generator

supervisor: supervisor.c common.h
	$(CC) $(CFLAGS) -o supervisor supervisor.c $(LDFLAGS)

generator: generator.c common.h
	$(CC) $(CFLAGS) -o generator generator.c $(LDFLAGS)

clean:
	rm -f supervisor generator
