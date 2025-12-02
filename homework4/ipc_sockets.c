#define _GNU_SOURCE
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "ipc.h"

int ipc_socket_server_create(const char *path) {
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        return -1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    unlink(path); /* remove old socket if exists */

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(fd);
        return -1;
    }

    if (listen(fd, 10) == -1) {
        perror("listen");
        close(fd);
        return -1;
    }

    return fd;
}

int ipc_socket_server_accept(int server_fd) {
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == -1) {
        if (errno == EINTR) return -1;
        perror("accept");
        return -1;
    }
    return client_fd;
}

int ipc_socket_client_connect(const char *path) {
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        return -1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect");
        close(fd);
        return -1;
    }

    return fd;
}
