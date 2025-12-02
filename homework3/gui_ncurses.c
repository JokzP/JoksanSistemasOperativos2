#include <stdio.h>
#include "scheduler.h"
#include "algorithms.h"
#include "metrics.h"

/* Stub / placeholder for ncurses-based UI.
 * For this assignment scaffold, we only provide a simple text-mode runner.
 */

int run_ncurses_ui(void) {
    printf("CPU Scheduler Simulator (text mode stub)\n");
    printf("This placeholder simply runs FIFO on a fixed workload.\n");

    process_t procs[3] = {
        { .pid = 1, .arrival_time = 0, .burst_time = 5, .priority = 1 },
        { .pid = 2, .arrival_time = 1, .burst_time = 3, .priority = 2 },
        { .pid = 3, .arrival_time = 2, .burst_time = 8, .priority = 1 }
    };
    for (int i = 0; i < 3; i++) {
        procs[i].remaining_time = procs[i].burst_time;
        procs[i].start_time = 0;
        procs[i].completion_time = 0;
    }

    timeline_event_t timeline[128];
    int tlen = 0;
    schedule_fifo(procs, 3, timeline, &tlen);

    int total_time = timeline[tlen-1].time + timeline[tlen-1].duration;
    int busy = total_time; /* no idle in this simple example */

    metrics_t m;
    calculate_metrics(procs, 3, total_time, busy, &m);

    printf("Timeline:\n");
    for (int i = 0; i < tlen; i++) {
        printf("  t=%d pid=%d dur=%d\n", timeline[i].time, timeline[i].pid, timeline[i].duration);
    }

    printf("Avg TAT=%.2f, Avg WT=%.2f, Avg RT=%.2f, CPU Util=%.1f%%, Throughput=%.2f, Fairness=%.2f\n",
           m.avg_turnaround_time, m.avg_waiting_time, m.avg_response_time,
           m.cpu_utilization, m.throughput, m.fairness_index);

    return 0;
}
