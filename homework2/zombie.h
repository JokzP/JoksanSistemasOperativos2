#ifndef ZOMBIE_H
#define ZOMBIE_H

#include <sys/types.h>

/* Initialize zombie prevention (call once at program start) */
void zombie_init(void);

/* Fork with automatic zombie prevention.
 * Returns: PID of child in parent, 0 in child, -1 on error.
 */
pid_t zombie_safe_fork(void);

/* Spawn command with zombie prevention.
 * Returns: child PID on success, -1 on failure.
 */
int zombie_safe_spawn(const char *command, char *args[]);

/* Zombie statistics */
typedef struct {
    int zombies_created;
    int zombies_reaped;
    int zombies_active;
} zombie_stats_t;

void zombie_get_stats(zombie_stats_t *stats);

#endif /* ZOMBIE_H */
