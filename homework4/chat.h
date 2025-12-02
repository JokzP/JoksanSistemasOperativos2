#ifndef CHAT_H
#define CHAT_H

#include <time.h>
#include <sys/types.h>

#include "message.h"

#define MAX_CLIENTS 64

typedef struct client {
    int fd;
    char username[USERNAME_MAX];
    time_t connected_at;
    struct client *next;
} client_t;

typedef struct {
    int total_messages;
    int active_users;
} server_stats_t;

/* Global server config */
extern const char *g_socket_path;

/* Server-side API */
void server_run(void);
void server_shutdown(void);

/* Client-side API */
int client_run(const char *username);

#endif /* CHAT_H */
