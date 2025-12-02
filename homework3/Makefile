CC=gcc
CFLAGS=-Wall -Wextra -g -Iinclude
LDFLAGS=

OBJS=src/scheduler.o src/algorithms.o src/metrics.o src/gui_ncurses.o src/gui_gtk.o src/report.o

TESTS=test_fifo test_sjf test_stcf test_rr test_mlfq

.PHONY: all clean tests

all: scheduler $(TESTS)

scheduler: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

src/scheduler.o: src/scheduler.c include/scheduler.h include/algorithms.h include/metrics.h
	$(CC) $(CFLAGS) -c $< -o $@

src/algorithms.o: src/algorithms.c include/algorithms.h include/scheduler.h
	$(CC) $(CFLAGS) -c $< -o $@

src/metrics.o: src/metrics.c include/metrics.h include/scheduler.h
	$(CC) $(CFLAGS) -c $< -o $@

src/gui_ncurses.o: src/gui_ncurses.c include/scheduler.h include/algorithms.h include/metrics.h
	$(CC) $(CFLAGS) -c $< -o $@

src/gui_gtk.o: src/gui_gtk.c
	$(CC) $(CFLAGS) -c $< -o $@

src/report.o: src/report.c include/scheduler.h include/algorithms.h include/metrics.h
	$(CC) $(CFLAGS) -c $< -o $@

test_%: tests/test_%.c src/algorithms.o src/metrics.o
	$(CC) $(CFLAGS) -o $@ tests/test_$*.c src/algorithms.o src/metrics.o $(LDFLAGS)

tests: $(TESTS)

clean:
	rm -f scheduler src/*.o $(TESTS)
