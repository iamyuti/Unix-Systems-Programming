CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g $(DEFS)

.PHONY: all clean

all: mygrep

mygrep: mygrep.c
	$(CC) $(CFLAGS) -o mygrep mygrep.c

clean:
	rm -f mygrep
