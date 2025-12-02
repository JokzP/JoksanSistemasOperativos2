#ifndef MESSAGE_H
#define MESSAGE_H

#include <time.h>

#define MSG_MAX_SIZE 256
#define USERNAME_MAX 32

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

typedef struct {
    message_type_t type;
    char from_user[USERNAME_MAX];
    char to_user[USERNAME_MAX];
    char text[MSG_MAX_SIZE];
    time_t timestamp;
    int client_id;
} message_t;

/* Serialization helpers for socket-based IPC */
int send_message_fd(int fd, const message_t *msg);
int recv_message_fd(int fd, message_t *msg);

/* Helpers to initialize messages */
void message_init(message_t *msg, message_type_t type, const char *from,
                  const char *to, const char *text);

#endif /* MESSAGE_H */
