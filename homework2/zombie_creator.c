#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/**
 * Creates N zombie processes for testing.
 * Parent does NOT call wait() immediately - children become zombies.
 * Returns: 0 on success, -1 on failure.
 */
int create_zombies(int count) {
    for (int i = 0; i < count; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return -1;
        }
        if (pid == 0) {
            /* child: exit immediately with unique code */
            _exit(i % 256);
        } else {
            printf("Created zombie: PID %d (exit code %d)\n", pid, i % 256);
        }
    }

    printf("Press Enter to exit and clean up zombies...\n");
    fflush(stdout);
    (void)getchar();

    /* Clean up zombies explicitly before exiting */
    int status;
    while (waitpid(-1, &status, 0) > 0) {
        /* reaping all children */
    }

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <num_zombies>\n", argv[0]);
        return EXIT_FAILURE;
    }
    int count = atoi(argv[1]);
    if (count <= 0) {
        fprintf(stderr, "Count must be positive.\n");
        return EXIT_FAILURE;
    }

    if (create_zombies(count) == -1) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
