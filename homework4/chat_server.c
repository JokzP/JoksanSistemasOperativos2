#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>

#include "chat.h"
#include "ipc.h"
#include "utils.h"

const char *g_socket_path = "/tmp/chat.sock";

static int server_fd = -1;
static client_t *client_list = NULL;
static pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;
static server_stats_t stats = {0, 0};
static volatile sig_atomic_t running = 1;

static void add_client(int fd);
static void remove_client_fd(int fd);
static client_t *find_client_by_fd(int fd);
static client_t *find_client_by_name(const char *name);
static void broadcast_message(const message_t *msg);
static void send_private_message(const message_t *msg, const char *to_user);
static void send_user_list(int fd);
static void handle_server_command(const char *line);
static void cleanup(void);

static void sigint_handler(int sig) {
    (void)sig;
    running = 0;
}

static void add_client(int fd) {
    client_t *c = calloc(1, sizeof(client_t));
    if (!c) {
        close(fd);
        return;
    }
    c->fd = fd;
    c->connected_at = time(NULL);
    strncpy(c->username, "unknown", USERNAME_MAX - 1);

    pthread_mutex_lock(&client_mutex);
    c->next = client_list;
    client_list = c;
    stats.active_users++;
    pthread_mutex_unlock(&client_mutex);

    log_msg("INFO", "Client connected (FD: %d)", fd);
}

static void remove_client_fd(int fd) {
    pthread_mutex_lock(&client_mutex);
    client_t *prev = NULL;
    client_t *cur = client_list;
    while (cur) {
        if (cur->fd == fd) {
            if (prev) prev->next = cur->next;
            else client_list = cur->next;
            if (stats.active_users > 0) stats.active_users--;
            log_msg("INFO", "Client '%s' disconnected (FD: %d)",
                    cur->username, fd);
            close(fd);
            free(cur);
            pthread_mutex_unlock(&client_mutex);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
    pthread_mutex_unlock(&client_mutex);
}

static client_t *find_client_by_fd(int fd) {
    client_t *c = client_list;
    while (c) {
        if (c->fd == fd) return c;
        c = c->next;
    }
    return NULL;
}

static client_t *find_client_by_name(const char *name) {
    if (!name) return NULL;
    client_t *c = client_list;
    while (c) {
        if (strcmp(c->username, name) == 0) return c;
        c = c->next;
    }
    return NULL;
}

static void broadcast_message(const message_t *msg) {
    pthread_mutex_lock(&client_mutex);
    client_t *c = client_list;
    while (c) {
        send_message_fd(c->fd, msg);
        c = c->next;
    }
    pthread_mutex_unlock(&client_mutex);
}

static void send_private_message(const message_t *msg, const char *to_user) {
    pthread_mutex_lock(&client_mutex);
    client_t *c = client_list;
    while (c) {
        if (strcmp(c->username, to_user) == 0) {
            send_message_fd(c->fd, msg);
            break;
        }
        c = c->next;
    }
    pthread_mutex_unlock(&client_mutex);
}

static void send_user_list(int fd) {
    message_t msg;
    message_init(&msg, MSG_USER_LIST, "server", "", "");
    /* build comma-separated list */
    char buf[MSG_MAX_SIZE] = {0};
    pthread_mutex_lock(&client_mutex);
    client_t *c = client_list;
    int first = 1;
    while (c) {
        if (!first) strncat(buf, ", ", sizeof(buf) - strlen(buf) - 1);
        strncat(buf, c->username, sizeof(buf) - strlen(buf) - 1);
        first = 0;
        c = c->next;
    }
    pthread_mutex_unlock(&client_mutex);
    strncpy(msg.text, buf, MSG_MAX_SIZE - 1);
    send_message_fd(fd, &msg);
}

static void handle_server_command(const char *line) {
    if (strncmp(line, "/list", 5) == 0) {
        pthread_mutex_lock(&client_mutex);
        client_t *c = client_list;
        log_msg("SERVER", "Connected users:");
        while (c) {
            log_msg("SERVER", " - %s (FD %d)", c->username, c->fd);
            c = c->next;
        }
        pthread_mutex_unlock(&client_mutex);
    } else if (strncmp(line, "/stats", 6) == 0) {
        log_msg("SERVER", "Total messages: %d, Active users: %d",
                stats.total_messages, stats.active_users);
    } else if (strncmp(line, "/kick ", 6) == 0) {
        const char *user = line + 6;
        while (*user == ' ') user++;
        pthread_mutex_lock(&client_mutex);
        client_t *prev = NULL;
        client_t *cur = client_list;
        while (cur) {
            if (strcmp(cur->username, user) == 0) {
                log_msg("SERVER", "Kicking user '%s'", user);
                if (prev) prev->next = cur->next;
                else client_list = cur->next;
                close(cur->fd);
                free(cur);
                if (stats.active_users > 0) stats.active_users--;
                break;
            }
            prev = cur;
            cur = cur->next;
        }
        pthread_mutex_unlock(&client_mutex);
    } else if (strncmp(line, "/broadcast ", 11) == 0) {
        const char *msg_text = line + 11;
        message_t msg;
        message_init(&msg, MSG_BROADCAST, "server", "ALL", msg_text);
        log_msg("MSG", "server -> ALL: %s", msg_text);
        broadcast_message(&msg);
    } else if (strncmp(line, "/shutdown", 9) == 0) {
        log_msg("SERVER", "Shutdown requested.");
        running = 0;
    } else {
        log_msg("SERVER", "Unknown command: %s", line);
    }
}

static void handle_client_message(int fd) {
    message_t msg;
    int r = recv_message_fd(fd, &msg);
    if (r == 0) {
        remove_client_fd(fd);
        return;
    }
    if (r < 0) {
        remove_client_fd(fd);
        return;
    }

    stats.total_messages++;

    if (msg.type == MSG_JOIN) {
        pthread_mutex_lock(&client_mutex);
        client_t *c = find_client_by_fd(fd);
        if (c) {
            strncpy(c->username, msg.from_user, USERNAME_MAX - 1);
            log_msg("INFO", "Client '%s' connected (FD: %d)",
                    c->username, fd);
        }
        pthread_mutex_unlock(&client_mutex);

        /* send welcome system message */
        message_t welcome;
        char buf[MSG_MAX_SIZE];
        snprintf(buf, sizeof(buf), "Welcome %s!", msg.from_user);
        message_init(&welcome, MSG_BROADCAST, "system", msg.from_user, buf);
        send_message_fd(fd, &welcome);

    } else if (msg.type == MSG_BROADCAST) {
        log_msg("MSG", "%s -> ALL: %s", msg.from_user, msg.text);
        broadcast_message(&msg);
    } else if (msg.type == MSG_PRIVATE) {
        log_msg("MSG", "%s -> %s: %s", msg.from_user, msg.to_user, msg.text);
        send_private_message(&msg, msg.to_user);
    } else if (msg.type == MSG_LIST_USERS) {
        send_user_list(fd);
    } else if (msg.type == MSG_LEAVE) {
        remove_client_fd(fd);
    }
}

void server_run(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    server_fd = ipc_socket_server_create(g_socket_path);
    if (server_fd == -1) {
        fprintf(stderr, "Failed to create server socket\n");
        exit(EXIT_FAILURE);
    }

    log_msg("SERVER", "Chat server started on %s", g_socket_path);
    log_msg("SERVER", "Using IPC method: UNIX_SOCKETS");

    while (running) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        FD_SET(STDIN_FILENO, &readfds);
        int maxfd = server_fd > STDIN_FILENO ? server_fd : STDIN_FILENO;

        pthread_mutex_lock(&client_mutex);
        client_t *c = client_list;
        while (c) {
            FD_SET(c->fd, &readfds);
            if (c->fd > maxfd) maxfd = c->fd;
            c = c->next;
        }
        pthread_mutex_unlock(&client_mutex);

        int rv = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if (rv == -1) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            char buf[512];
            if (!fgets(buf, sizeof(buf), stdin)) {
                running = 0;
            } else {
                trim_newline(buf);
                if (buf[0] == '/') {
                    handle_server_command(buf);
                }
            }
        }

        if (FD_ISSET(server_fd, &readfds)) {
            int client_fd = ipc_socket_server_accept(server_fd);
            if (client_fd != -1) {
                add_client(client_fd);
            }
        }

        pthread_mutex_lock(&client_mutex);
        c = client_list;
        while (c) {
            int fd = c->fd;
            client_t *next = c->next;
            if (FD_ISSET(fd, &readfds)) {
                pthread_mutex_unlock(&client_mutex);
                handle_client_message(fd);
                pthread_mutex_lock(&client_mutex);
                c = client_list; /* restart as list may have changed */
                continue;
            }
            c = next;
        }
        pthread_mutex_unlock(&client_mutex);
    }

    cleanup();
}

static void cleanup(void) {
    log_msg("SERVER", "Shutting down server...");
    pthread_mutex_lock(&client_mutex);
    client_t *c = client_list;
    while (c) {
        close(c->fd);
        client_t *next = c->next;
        free(c);
        c = next;
    }
    client_list = NULL;
    pthread_mutex_unlock(&client_mutex);

    if (server_fd != -1) {
        close(server_fd);
        unlink(g_socket_path);
    }

    log_msg("SERVER", "Total messages: %d, Active users: %d",
            stats.total_messages, stats.active_users);
}

int main(void) {
    server_run();
    return 0;
}
