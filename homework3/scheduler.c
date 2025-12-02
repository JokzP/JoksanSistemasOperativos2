#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scheduler.h"
#include "algorithms.h"
#include "metrics.h"

int run_ncurses_ui(void); /* from gui_ncurses.c */
void generate_report(const char *filename, process_t *processes, int n);

static int load_workload(const char *filename, process_t *procs, int *n) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("fopen workload");
        return -1;
    }
    int count = 0;
    while (!feof(f) && count < MAX_PROCESSES) {
        int pid, at, bt, prio;
        if (fscanf(f, "%d %d %d %d", &pid, &at, &bt, &prio) == 4) {
            procs[count].pid = pid;
            procs[count].arrival_time = at;
            procs[count].burst_time = bt;
            procs[count].priority = prio;
            procs[count].remaining_time = bt;
            procs[count].start_time = 0;
            procs[count].completion_time = 0;
            count++;
        } else {
            break;
        }
    }
    fclose(f);
    *n = count;
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <workload_file>\n", argv[0]);
        printf("       %s --demo-ui   (run simple text UI)\n", argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "--demo-ui") == 0) {
        return run_ncurses_ui();
    }

    process_t procs[MAX_PROCESSES];
    int n = 0;
    if (load_workload(argv[1], procs, &n) == -1) {
        return 1;
    }

    printf("Loaded %d processes from %s\n", n, argv[1]);

    generate_report("report.md", procs, n);
    printf("Generated report.md with comparison of algorithms.\n");

    return 0;
}
