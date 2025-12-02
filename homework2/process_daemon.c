#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

static volatile sig_atomic_t daemon_running = 1;

void daemon_sigterm_handler(int sig) {
    (void)sig;
    daemon_running = 0;
}

void daemon_sigchld_handler(int sig) {
    (void)sig;
    int saved_errno = errno;
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        (void)pid;
    }
    errno = saved_errno;
}

/**
 * Daemonize process - become background daemon
 */
void daemonize(void) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS); /* parent */
    }

    if (setsid() == -1) {
        perror("setsid");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid < 0) {
        perror("fork2");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    umask(0);
    chdir("/");

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    int fd = open("/dev/null", O_RDWR);
    if (fd >= 0) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if (fd > 2) close(fd);
    }
}

/**
 * Spawn worker processes periodically
 */
void spawn_workers(void) {
    int log_fd = open("/tmp/daemon.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (log_fd == -1) {
        /* logging failure is non-fatal for demo */
    }

    while (daemon_running) {
        pid_t pid = fork();
        if (pid < 0) {
            /* could log error */
        } else if (pid == 0) {
            /* worker */
            char buf[256];
            time_t now = time(NULL);
            struct tm tm_now;
            localtime_r(&now, &tm_now);
            int len = snprintf(buf, sizeof(buf),
                               "[%04d-%02d-%02d %02d:%02d:%02d] Worker PID %d: doing work\n",
                               tm_now.tm_year + 1900,
                               tm_now.tm_mon + 1,
                               tm_now.tm_mday,
                               tm_now.tm_hour,
                               tm_now.tm_min,
                               tm_now.tm_sec,
                               getpid());
            int fdw = open("/tmp/daemon.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fdw != -1) {
                write(fdw, buf, len);
                close(fdw);
            }
            sleep(2);
            _exit(0);
        }

        sleep(5);
    }

    if (log_fd != -1) {
        close(log_fd);
    }
}

int main(void) {
    daemonize();

    struct sigaction sa_term;
    memset(&sa_term, 0, sizeof(sa_term));
    sa_term.sa_handler = daemon_sigterm_handler;
    sigemptyset(&sa_term.sa_mask);
    sigaction(SIGTERM, &sa_term, NULL);

    struct sigaction sa_chld;
    memset(&sa_chld, 0, sizeof(sa_chld));
    sa_chld.sa_handler = daemon_sigchld_handler;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa_chld, NULL);

    spawn_workers();

    return 0;
}
