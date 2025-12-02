#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>
#include <signal.h>

#include "chat.h"
#include "ipc.h"
#include "utils.h"

static volatile sig_atomic_t running_client = 1;

static void sigint_handler_client(int sig) {
    (void)sig;
    running_client = 0;
}

static void print_help(void) {
    printf("Available commands:\n");
    printf("  /help                 - Show this help\n");
    printf("  /users                - List online users\n");
    printf("  /msg <user> <msg>     - Send private message\n");
    printf("  /quit                 - Disconnect\n");
}

int client_run(const char *username) {
    int fd = ipc_socket_client_connect("/tmp/chat.sock");
    if (fd == -1) {
        fprintf(stderr, "Could not connect to server.\n");
        return -1;
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler_client;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    printf("=== Chat Client ===\n");
    printf("Username: %s\n", username);
    printf("Connected to server!\n\n");

    /* send join message */
    message_t join;
    message_init(&join, MSG_JOIN, username, "", "");
    send_message_fd(fd, &join);

    print_help();

    char input[512];

    while (running_client) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(fd, &readfds);
        int maxfd = fd > STDIN_FILENO ? fd : STDIN_FILENO;

        int rv = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if (rv == -1) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }

        if (FD_ISSET(fd, &readfds)) {
            message_t msg;
            int r = recv_message_fd(fd, &msg);
            if (r == 0) {
                printf("[SYSTEM] Server disconnected.\n");
                break;
            } else if (r < 0) {
                printf("[SYSTEM] Error receiving from server.\n");
                break;
            } else {
                if (msg.type == MSG_BROADCAST) {
                    if (strcmp(msg.from_user, username) == 0) {
                        printf("[you] %s\n", msg.text);
                    } else {
                        printf("[%s] %s\n", msg.from_user, msg.text);
                    }
                } else if (msg.type == MSG_PRIVATE) {
                    if (strcmp(msg.to_user, username) == 0) {
                        printf("[%s -> you] %s\n", msg.from_user, msg.text);
                    } else if (strcmp(msg.from_user, username) == 0) {
                        printf("[You -> %s] %s\n", msg.to_user, msg.text);
                    }
                } else if (msg.type == MSG_USER_LIST) {
                    printf("[SYSTEM] Users online: %s\n", msg.text);
                } else if (msg.type == MSG_ERROR) {
                    printf("[ERROR] %s\n", msg.text);
                }
            }
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            if (!fgets(input, sizeof(input), stdin)) {
                running_client = 0;
                break;
            }
            trim_newline(input);
            if (strlen(input) == 0) continue;

            if (input[0] == '/') {
                if (strcmp(input, "/help") == 0) {
                    print_help();
                } else if (strcmp(input, "/users") == 0) {
                    message_t req;
                    message_init(&req, MSG_LIST_USERS, username, "", "");
                    send_message_fd(fd, &req);
                } else if (strncmp(input, "/msg ", 5) == 0) {
                    char *rest = input + 5;
                    char *user = strtok(rest, " ");
                    char *msg_text = strtok(NULL, "");
                    if (!user || !msg_text) {
                        printf("Usage: /msg <user> <msg>\n");
                    } else {
                        message_t pm;
                        message_init(&pm, MSG_PRIVATE, username, user, msg_text);
                        send_message_fd(fd, &pm);
                        printf("[You -> %s] %s\n", user, msg_text);
                    }
                } else if (strcmp(input, "/quit") == 0) {
                    message_t leave;
                    message_init(&leave, MSG_LEAVE, username, "", "bye");
                    send_message_fd(fd, &leave);
                    running_client = 0;
                    printf("[SYSTEM] Goodbye!\n");
                    break;
                } else {
                    printf("Unknown command. Type /help.\n");
                }
            } else {
                message_t msg;
                message_init(&msg, MSG_BROADCAST, username, "ALL", input);
                send_message_fd(fd, &msg);
            }
        }
    }

    close(fd);
    return 0;
}

int main(void) {
    char username[USERNAME_MAX];
    printf("=== Chat Client ===\n");
    printf("Username: ");
    fflush(stdout);
    if (!fgets(username, sizeof(username), stdin)) {
        return 0;
    }
    trim_newline(username);
    if (strlen(username) == 0) {
        fprintf(stderr, "Username cannot be empty.\n");
        return 1;
    }
    client_run(username);
    return 0;
}
