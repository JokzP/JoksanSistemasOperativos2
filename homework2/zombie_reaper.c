#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

/* Strategy 1: Explicit wait */
void reap_explicit(void) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, 0)) > 0) {
        (void)pid;
        /* Reaped child explicitly */
    }
    if (pid == -1 && errno != ECHILD) {
        perror("waitpid");
    }
}

/* Strategy 2: SIGCHLD handler */
static void sigchld_handler_int(int sig) {
    (void)sig;
    int saved_errno = errno;
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        (void)pid;
        /* child reaped in handler */
    }
    errno = saved_errno;
}

void setup_auto_reaper(void) {
    struct sigaction sa;
    sa.sa_handler = sigchld_handler_int;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction(SIGCHLD)");
    }
}

/* Strategy 3: Ignore SIGCHLD */
void setup_ignore_reaper(void) {
    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
        perror("signal(SIGCHLD, SIG_IGN)");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <strategy(1|2|3)>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int strategy = atoi(argv[1]);
    if (strategy < 1 || strategy > 3) {
        fprintf(stderr, "Strategy must be 1, 2, or 3.\n");
        return EXIT_FAILURE;
    }

    /* Install SIGCHLD handler by default so strategy 2 works reliably.
       Strategy 3 (ignore) will override this handler. */
    setup_auto_reaper();

    /* Create 10 children */
    for (int i = 0; i < 10; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return EXIT_FAILURE;
        }
        if (pid == 0) {
            /* child */
            srand(getpid());
            sleep(rand() % 3);
            _exit(i);
        }
    }

    /* Use chosen strategy */
    switch (strategy) {
        case 1:
            reap_explicit();
            break;
        case 2:
            /* handler already installed */
            break;
        case 3:
            setup_ignore_reaper();
            break;
    }

    sleep(5);  /* wait for all children to exit */

    /* Verify no zombies (manual inspection) */
    system("ps aux | grep Z | grep -v grep");

    return EXIT_SUCCESS;
}
