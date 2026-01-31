CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -Wextra -D_POSIX_C_SOURCE=200809L -g

.PHONY: all clean

all: forksort

forksort: forksort.c
	$(CC) $(CFLAGS) -o forksort forksort.c

clean:
	rm -f forksort
