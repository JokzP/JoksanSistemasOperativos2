#include <stdio.h>
#include "../include/scheduler.h"
#include "../include/algorithms.h"
#include "../include/metrics.h"

int main(void) {
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

    mlfq_config_t cfg; int qs[3]={2,4,8}; cfg.num_queues=3; cfg.quantums=qs; cfg.boost_interval=20; schedule_mlfq(procs, 3, &cfg, timeline, &tlen);

    int total_time = timeline[tlen-1].time + timeline[tlen-1].duration;
    int busy = total_time;
    metrics_t m;
    calculate_metrics(procs, 3, total_time, busy, &m);

    printf("Avg TAT=%.2f, Avg WT=%.2f, Avg RT=%.2f, Throughput=%.2f\n",
           m.avg_turnaround_time, m.avg_waiting_time, m.avg_response_time,
           m.throughput);
    return 0;
}
