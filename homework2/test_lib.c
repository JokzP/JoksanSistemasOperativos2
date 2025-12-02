#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "zombie.h"

int main(void) {
    zombie_init();

    char *args[] = { "sleep", "2", NULL };

    for (int i = 0; i < 3; i++) {
        int pid = zombie_safe_spawn("sleep", args);
        if (pid < 0) {
            fprintf(stderr, "Failed to spawn child %d\n", i);
        } else {
            printf("Spawned child PID %d\n", pid);
        }
    }

    printf("Waiting for children to exit...\n");
    sleep(3);

    zombie_stats_t stats;
    zombie_get_stats(&stats);

    printf("Zombie stats:\n");
    printf("  Created: %d\n", stats.zombies_created);
    printf("  Reaped : %d\n", stats.zombies_reaped);
    printf("  Active : %d\n", stats.zombies_active);

    return 0;
}
