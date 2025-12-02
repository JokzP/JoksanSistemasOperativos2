CC=gcc
CFLAGS=-Wall -Wextra -g -Isrc
LDFLAGS=
LIB=libzombie.a

SRC_DIR=src
TEST_DIR=tests

CREATOR=$(SRC_DIR)/zombie_creator
DETECTOR=$(SRC_DIR)/zombie_detector
REAPER=$(SRC_DIR)/zombie_reaper
DAEMON=$(SRC_DIR)/process_daemon
TEST_LIB_BIN=$(SRC_DIR)/test_lib

BINS=zombie_creator zombie_detector zombie_reaper process_daemon test_lib

.PHONY: all clean test

all: $(LIB) $(BINS)

$(LIB): src/zombie.o
	ar rcs $(LIB) src/zombie.o

src/zombie.o: src/zombie.c src/zombie.h
	$(CC) $(CFLAGS) -c src/zombie.c -o src/zombie.o

zombie_creator: src/zombie_creator.c
	$(CC) $(CFLAGS) -o $@ $<

zombie_detector: src/zombie_detector.c
	$(CC) $(CFLAGS) -o $@ $<

zombie_reaper: src/zombie_reaper.c
	$(CC) $(CFLAGS) -o $@ $<

process_daemon: src/process_daemon.c
	$(CC) $(CFLAGS) -o $@ $<

test_lib: src/test_lib.c $(LIB)
	$(CC) $(CFLAGS) -o $@ src/test_lib.c -L. -lzombie

test: all
	cd $(TEST_DIR) && ./test_creator.sh && ./test_detector.sh && ./test_reaper.sh && ./test_daemon.sh

clean:
	rm -f $(BINS) $(LIB) src/*.o
