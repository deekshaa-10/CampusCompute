CC = gcc
CFLAGS = -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -O2 -pthread
OBJS = main.o jobs.o scheduler.o banker.o logger.o ipc.o worker.o limits.o

all: campuscompute
campuscompute: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) -pthread
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f $(OBJS) campuscompute
.PHONY: all clean
