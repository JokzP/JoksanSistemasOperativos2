#define _GNU_SOURCE
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#include "message.h"

static int write_all(int fd, const void *buf, size_t len) {
    const char *p = buf;
    size_t total = 0;
    while (total < len) {
        ssize_t n = write(fd, p + total, len - total);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (n == 0) break;
        total += (size_t)n;
    }
    return (int)total;
}

static int read_all(int fd, void *buf, size_t len) {
    char *p = buf;
    size_t total = 0;
    while (total < len) {
        ssize_t n = read(fd, p + total, len - total);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (n == 0) {
            /* EOF */
            break;
        }
        total += (size_t)n;
    }
    if (total == 0) return 0;
    return (int)total;
}

int send_message_fd(int fd, const message_t *msg) {
    if (!msg) return -1;
    message_t tmp = *msg;
    if (tmp.timestamp == 0) {
        tmp.timestamp = time(NULL);
    }
    int n = write_all(fd, &tmp, sizeof(tmp));
    return (n == sizeof(tmp)) ? 0 : -1;
}

int recv_message_fd(int fd, message_t *msg) {
    if (!msg) return -1;
    int n = read_all(fd, msg, sizeof(*msg));
    if (n == 0) {
        /* disconnected */
        return 0;
    }
    if (n != sizeof(*msg)) {
        return -1;
    }
    return 1;
}

void message_init(message_t *msg, message_type_t type, const char *from,
                  const char *to, const char *text) {
    if (!msg) return;
    memset(msg, 0, sizeof(*msg));
    msg->type = type;
    if (from) {
        strncpy(msg->from_user, from, USERNAME_MAX - 1);
    }
    if (to) {
        strncpy(msg->to_user, to, USERNAME_MAX - 1);
    }
    if (text) {
        strncpy(msg->text, text, MSG_MAX_SIZE - 1);
    }
    msg->timestamp = time(NULL);
}
