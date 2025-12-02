# ARCHITECTURE.md - System Design

## Overview

The system consists of:

- A single-threaded `chat_server` managing multiple clients.
- Multiple `chat_client` instances connecting via UNIX domain sockets.

## Server

- Creates a UNIX domain socket at `/tmp/chat.sock`.
- Uses `select()` to handle:
  - New client connections on the listening socket.
  - Incoming messages from clients.
  - Server commands from stdin.

Clients are stored in a linked list:

```c
typedef struct client {
    int fd;
    char username[USERNAME_MAX];
    time_t connected_at;
    struct client *next;
} client_t;
```

A `pthread_mutex_t` protects the list for future extensibility (if threads were added).

### Server Commands

Read from stdin:

- `/list` – prints list of connected users.
- `/stats` – prints total messages and active users.
- `/kick <username>` – removes a specific user.
- `/broadcast <msg>` – sends a server-originated broadcast.
- `/shutdown` – stops the main loop and gracefully closes all sockets.

## Client

- Connects to server socket.
- Sends `MSG_JOIN` with chosen username.
- Uses `select()` to multiplex between:
  - Stdin (user input).
  - Server socket (incoming messages).

Commands:

- `/help`, `/users`, `/msg <user> <msg>`, `/quit`.

Normal text is sent as `MSG_BROADCAST`.

## IPC Modules

- `ipc_sockets.c`: wrapper around UNIX domain sockets for server/client.
- `ipc_mq.c`: POSIX message queue wrapper for optional/alternate IPC.

## Synchronization

- `client_list` is guarded by a `pthread_mutex_t`.
- The server loop is otherwise single-threaded, so concurrency issues are minimal.
