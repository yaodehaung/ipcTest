# Makefile for IPC Email/Timer Example

CC = gcc
CFLAGS = -Wall -O2
TARGETS = main_email main_timer exit_broadcast
SRCS = main_email.c main_timer.c exit_broadcast.c
COMMON = ipc_common.h

all: $(TARGETS)

main_email: main_email.c $(COMMON)
	$(CC) $(CFLAGS) -o $@ main_email.c

main_timer: main_timer.c $(COMMON)
	$(CC) $(CFLAGS) -o $@ main_timer.c

exit_broadcast: exit_broadcast.c $(COMMON)
	$(CC) $(CFLAGS) -o $@ exit_broadcast.c

clean:
	rm -f $(TARGETS) *.o

.PHONY: all clean

