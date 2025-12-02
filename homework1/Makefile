CC=gcc
CFLAGS=-Wall -Wextra -g
LDFLAGS=

TARGET=procman
SRCS=procman.c
OBJS=$(SRCS:.c=.o)

.PHONY: all clean test valgrind strace

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.c procman.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

test: all
	cd test_scripts && ./test1.sh && ./test2.sh && ./test3.sh

valgrind: all
	valgrind --leak-check=full ./$(TARGET)

strace: all
	strace -f -e trace=fork,execve,wait4,kill ./$(TARGET)
