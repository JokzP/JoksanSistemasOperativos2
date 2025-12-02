#define _GNU_SOURCE
#include <mqueue.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "ipc.h"

int ipc_mq_open_server(const char *name) {
    struct mq_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 512;
    mqd_t mq = mq_open(name, O_CREAT | O_RDWR, 0644, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open server");
        return -1;
    }
    return (int)mq;
}

int ipc_mq_open_client(const char *name) {
    mqd_t mq = mq_open(name, O_RDWR);
    if (mq == (mqd_t)-1) {
        perror("mq_open client");
        return -1;
    }
    return (int)mq;
}

int ipc_mq_send(int mq, const void *buf, size_t len) {
    if (mq_send((mqd_t)mq, buf, len, 0) == -1) {
        perror("mq_send");
        return -1;
    }
    return 0;
}

int ipc_mq_recv(int mq, void *buf, size_t len) {
    ssize_t n = mq_receive((mqd_t)mq, buf, len, NULL);
    if (n == -1) {
        perror("mq_receive");
        return -1;
    }
    return (int)n;
}
