#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

/**
 * Scans /proc filesystem for zombie processes.
 * zombie_pids: array to store found PIDs (may be NULL to just count).
 * max_zombies: max number to store in array.
 * Returns: number of zombies found.
 */
int find_zombies(int *zombie_pids, int max_zombies) {
    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("opendir /proc");
        return -1;
    }

    struct dirent *de;
    int count = 0;

    while ((de = readdir(dir)) != NULL) {
        char *endptr = NULL;
        long pid_long = strtol(de->d_name, &endptr, 10);
        if (*endptr != '\0') {
            continue; /* ignore non-numeric */
        }
        if (pid_long <= 0) continue;
        pid_t pid = (pid_t)pid_long;

        char path[256];
        snprintf(path, sizeof(path), "/proc/%d/stat", pid);
        FILE *fp = fopen(path, "r");
        if (!fp) {
            continue;
        }

        int file_pid = 0;
        char comm[256] = {0};
        char state = 0;
        int ppid = 0;

        if (fscanf(fp, "%d %255s %c %d", &file_pid, comm, &state, &ppid) != 4) {
            fclose(fp);
            continue;
        }
        fclose(fp);

        if (state == 'Z') {
            if (zombie_pids && count < max_zombies) {
                zombie_pids[count] = file_pid;
            }
            count++;
        }
    }

    closedir(dir);
    return count;
}

/**
 * Prints information about a zombie process.
 */
void print_zombie_info(int pid) {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    FILE *fp = fopen(path, "r");
    if (!fp) {
        perror("fopen stat");
        return;
    }

    int file_pid = 0;
    char comm[256] = {0};
    char state = 0;
    int ppid = 0;

    if (fscanf(fp, "%d %255s %c %d", &file_pid, comm, &state, &ppid) != 4) {
        fclose(fp);
        return;
    }
    fclose(fp);

    /* Remove parentheses from comm if present */
    size_t len = strlen(comm);
    if (len > 2 && comm[0] == '(' && comm[len-1] == ')') {
        memmove(comm, comm+1, len-2);
        comm[len-2] = '\0';
    }

    /* Time column placeholder (00:00:00) */
    const char *time_str = "00:00:00";

    printf("%-7d%-8d%-15s%-7c%s\n", file_pid, ppid, comm, state, time_str);
}

int main(void) {
    int zombie_pids[1024];
    int total = find_zombies(zombie_pids, 1024);
    if (total < 0) {
        fprintf(stderr, "Error scanning for zombies.\n");
        return EXIT_FAILURE;
    }

    printf("=== Zombie Process Report ===\n");
    printf("Total Zombies: %d\n\n", total);
    if (total == 0) {
        return EXIT_SUCCESS;
    }

    printf("PID     PPID    Command         State   Time\n");
    printf("-----   -----   -------------   -----   --------\n");

    for (int i = 0; i < total && i < 1024; i++) {
        print_zombie_info(zombie_pids[i]);
    }

    /* Simple parent process analysis: count zombies per PPID */
    /* For brevity, we recompute parent for each PID */
    printf("\nParent Process Analysis:\n");

    int counted[1024];
    int parent_ids[1024];
    int parent_counts[1024];
    int parent_total = 0;

    for (int i = 0; i < total && i < 1024; i++) {
        char path[256];
        snprintf(path, sizeof(path), "/proc/%d/stat", zombie_pids[i]);
        FILE *fp = fopen(path, "r");
        if (!fp) continue;

        int file_pid = 0;
        char comm[256] = {0};
        char state = 0;
        int ppid = 0;
        if (fscanf(fp, "%d %255s %c %d", &file_pid, comm, &state, &ppid) != 4) {
            fclose(fp);
            continue;
        }
        fclose(fp);

        int found = -1;
        for (int j = 0; j < parent_total; j++) {
            if (parent_ids[j] == ppid) {
                found = j;
                break;
            }
        }
        if (found == -1 && parent_total < 1024) {
            parent_ids[parent_total] = ppid;
            parent_counts[parent_total] = 1;
            parent_total++;
        } else if (found != -1) {
            parent_counts[found]++;
        }
    }

    for (int i = 0; i < parent_total; i++) {
        printf("  PID %d has %d zombie children\n", parent_ids[i], parent_counts[i]);
    }

    return EXIT_SUCCESS;
}
