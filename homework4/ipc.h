#ifndef IPC_H
#define IPC_H

#include <sys/types.h>

/* UNIX domain socket IPC (used by server and client) */
int ipc_socket_server_create(const char *path);
int ipc_socket_server_accept(int server_fd);
int ipc_socket_client_connect(const char *path);

/* POSIX message queue IPC (second IPC method for assignment) */
int ipc_mq_open_server(const char *name);
int ipc_mq_open_client(const char *name);
int ipc_mq_send(int mq, const void *buf, size_t len);
int ipc_mq_recv(int mq, void *buf, size_t len);

#endif /* IPC_H */
