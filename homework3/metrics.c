#include <math.h>
#include "metrics.h"

void calculate_metrics(process_t *processes, int n, int total_time, int busy_time,
                       metrics_t *metrics) {
    double sum_tat = 0.0;
    double sum_wt = 0.0;
    double sum_rt = 0.0;
    double sum = 0.0;
    double sum_sq = 0.0;

    for (int i = 0; i < n; i++) {
        processes[i].turnaround_time = processes[i].completion_time - processes[i].arrival_time;
        processes[i].waiting_time = processes[i].turnaround_time - processes[i].burst_time;
        processes[i].response_time = processes[i].start_time - processes[i].arrival_time;

        double x = processes[i].turnaround_time;
        sum_tat += processes[i].turnaround_time;
        sum_wt += processes[i].waiting_time;
        sum_rt += processes[i].response_time;

        sum += x;
        sum_sq += x * x;
    }

    metrics->avg_turnaround_time = sum_tat / n;
    metrics->avg_waiting_time = sum_wt / n;
    metrics->avg_response_time = sum_rt / n;
    metrics->cpu_utilization = (total_time > 0) ? (100.0 * busy_time / total_time) : 0.0;
    metrics->throughput = (total_time > 0) ? ((double)n / total_time) : 0.0;

    if (n > 0 && sum_sq > 0) {
        metrics->fairness_index = (sum * sum) / (n * sum_sq);
    } else {
        metrics->fairness_index = 0.0;
    }
}
