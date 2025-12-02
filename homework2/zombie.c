#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#include "zombie.h"

static zombie_stats_t g_stats = {0, 0, 0};
static pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;
static int initialized = 0;

static void zombie_sigchld_handler(int sig) {
    (void)sig;
    int saved_errno = errno;
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        (void)pid;
        pthread_mutex_lock(&stats_mutex);
        g_stats.zombies_reaped++;
        if (g_stats.zombies_active > 0) {
            g_stats.zombies_active--;
        }
        pthread_mutex_unlock(&stats_mutex);
    }

    errno = saved_errno;
}

void zombie_init(void) {
    if (initialized) return;

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = zombie_sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction(SIGCHLD)");
        return;
    }

    initialized = 1;
}

pid_t zombie_safe_fork(void) {
    if (!initialized) {
        zombie_init();
    }

    pid_t pid = fork();
    if (pid > 0) {
        /* parent */
        pthread_mutex_lock(&stats_mutex);
        g_stats.zombies_created++;
        g_stats.zombies_active++;
        pthread_mutex_unlock(&stats_mutex);
    }
    return pid;
}

int zombie_safe_spawn(const char *command, char *args[]) {
    pid_t pid = zombie_safe_fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    if (pid == 0) {
        /* child */
        execvp(command, args);
        perror("execvp");
        _exit(127);
    }
    /* parent */
    return pid;
}

void zombie_get_stats(zombie_stats_t *stats) {
    if (!stats) return;
    pthread_mutex_lock(&stats_mutex);
    *stats = g_stats;
    pthread_mutex_unlock(&stats_mutex);
}
