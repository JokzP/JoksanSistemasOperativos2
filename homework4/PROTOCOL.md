# PROTOCOL.md - Chat Message Protocol

## Message Types

```c
typedef enum {
    MSG_JOIN,
    MSG_LEAVE,
    MSG_BROADCAST,
    MSG_PRIVATE,
    MSG_LIST_USERS,
    MSG_USER_LIST,
    MSG_ERROR,
    MSG_PING,
    MSG_PONG
} message_type_t;
```

## Message Format

```c
#define MSG_MAX_SIZE 256
#define USERNAME_MAX 32

typedef struct {
    message_type_t type;
    char from_user[USERNAME_MAX];
    char to_user[USERNAME_MAX];
    char text[MSG_MAX_SIZE];
    time_t timestamp;
    int client_id;
} message_t;
```

Messages are sent as fixed-size structs over the UNIX domain socket.

## Semantics

- `MSG_JOIN`: sent by client when connecting. `from_user` contains the username.
- `MSG_LEAVE`: sent by client when disconnecting.
- `MSG_BROADCAST`: message to all users. `text` contains the content.
- `MSG_PRIVATE`: private message. `to_user` is the recipient, `text` the body.
- `MSG_LIST_USERS`: client requests a list of users.
- `MSG_USER_LIST`: server response, `text` contains comma-separated usernames.
- `MSG_ERROR`: server-to-client error messages.
- `MSG_PING`/`MSG_PONG`: reserved for future keep-alive implementation.
