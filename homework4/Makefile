CC=gcc
CFLAGS=-Wall -Wextra -g -Isrc -Iinclude
LDFLAGS=-lpthread -lrt

SRV_OBJS=src/chat_server.o src/ipc_sockets.o src/ipc_mq.o src/message.o src/utils.o
CLI_OBJS=src/chat_client.o src/ipc_sockets.o src/ipc_mq.o src/message.o src/utils.o

TARGET_SERVER=chat_server
TARGET_CLIENT=chat_client

.PHONY: all clean test

all: $(TARGET_SERVER) $(TARGET_CLIENT)

$(TARGET_SERVER): $(SRV_OBJS)
	$(CC) $(CFLAGS) -o $@ $(SRV_OBJS) $(LDFLAGS)

$(TARGET_CLIENT): $(CLI_OBJS)
	$(CC) $(CFLAGS) -o $@ $(CLI_OBJS) $(LDFLAGS)

src/%.o: src/%.c include/%.h
	$(CC) $(CFLAGS) -c $< -o $@

src/chat_server.o: src/chat_server.c include/chat.h include/message.h include/ipc.h include/utils.h
	$(CC) $(CFLAGS) -c $< -o $@

src/chat_client.o: src/chat_client.c include/chat.h include/message.h include/ipc.h include/utils.h
	$(CC) $(CFLAGS) -c $< -o $@

src/message.o: src/message.c include/message.h
	$(CC) $(CFLAGS) -c $< -o $@

src/ipc_sockets.o: src/ipc_sockets.c include/ipc.h
	$(CC) $(CFLAGS) -c $< -o $@

src/ipc_mq.o: src/ipc_mq.c include/ipc.h
	$(CC) $(CFLAGS) -c $< -o $@

src/utils.o: src/utils.c include/utils.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET_SERVER) $(TARGET_CLIENT) src/*.o

test: all
	cd tests && ./test_server.sh & sleep 1 && ./test_client.sh
