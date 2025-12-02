#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>

#include "algorithms.h"

static int all_done(process_t *p, int n) {
    for (int i = 0; i < n; i++) {
        if (p[i].remaining_time > 0) return 0;
    }
    return 1;
}

/* ---------- FIFO ---------- */

void schedule_fifo(process_t *processes, int n, timeline_event_t *timeline, int *timeline_len) {
    int time = 0;
    int tlen = 0;

    /* sort by arrival_time then pid (stable) */
    process_t *p = malloc(sizeof(process_t)*n);
    memcpy(p, processes, sizeof(process_t)*n);

    for (int i = 0; i < n; i++) {
        for (int j = i+1; j < n; j++) {
            if (p[j].arrival_time < p[i].arrival_time ||
               (p[j].arrival_time == p[i].arrival_time && p[j].pid < p[i].pid)) {
                process_t tmp = p[i]; p[i] = p[j]; p[j] = tmp;
            }
        }
    }

    for (int i = 0; i < n; i++) {
        if (time < p[i].arrival_time) {
            time = p[i].arrival_time;
        }
        p[i].start_time = time;
        timeline[tlen].time = time;
        timeline[tlen].pid = p[i].pid;
        timeline[tlen].duration = p[i].burst_time;
        tlen++;

        time += p[i].burst_time;
        p[i].completion_time = time;
        p[i].remaining_time = 0;
    }

    /* write back computed times */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (processes[j].pid == p[i].pid) {
                processes[j].start_time = p[i].start_time;
                processes[j].completion_time = p[i].completion_time;
                processes[j].remaining_time = 0;
            }
        }
    }

    *timeline_len = tlen;
    free(p);
}

/* ---------- SJF (non-preemptive) ---------- */

void schedule_sjf(process_t *processes, int n, timeline_event_t *timeline, int *timeline_len) {
    process_t *p = malloc(sizeof(process_t)*n);
    memcpy(p, processes, sizeof(process_t)*n);
    for (int i = 0; i < n; i++) p[i].remaining_time = p[i].burst_time;

    int time = 0;
    int completed = 0;
    int tlen = 0;

    while (completed < n) {
        int idx = -1;
        int best_bt = INT_MAX;
        for (int i = 0; i < n; i++) {
            if (p[i].remaining_time > 0 && p[i].arrival_time <= time) {
                if (p[i].burst_time < best_bt) {
                    best_bt = p[i].burst_time;
                    idx = i;
                }
            }
        }
        if (idx == -1) {
            time++;
            continue;
        }
        if (p[idx].start_time == 0 && time >= p[idx].arrival_time)
            p[idx].start_time = time;

        timeline[tlen].time = time;
        timeline[tlen].pid = p[idx].pid;
        timeline[tlen].duration = p[idx].burst_time;
        tlen++;

        time += p[idx].burst_time;
        p[idx].completion_time = time;
        p[idx].remaining_time = 0;
        completed++;
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (processes[j].pid == p[i].pid) {
                processes[j].start_time = p[i].start_time;
                processes[j].completion_time = p[i].completion_time;
                processes[j].remaining_time = 0;
            }
        }
    }

    *timeline_len = tlen;
    free(p);
}

/* ---------- STCF (preemptive SJF) ---------- */

void schedule_stcf(process_t *processes, int n, timeline_event_t *timeline, int *timeline_len) {
    process_t *p = malloc(sizeof(process_t)*n);
    memcpy(p, processes, sizeof(process_t)*n);
    for (int i = 0; i < n; i++) p[i].remaining_time = p[i].burst_time;

    int time = 0;
    int tlen = 0;
    int last_pid = -1;

    while (!all_done(p, n)) {
        int idx = -1;
        int best_rt = INT_MAX;
        for (int i = 0; i < n; i++) {
            if (p[i].remaining_time > 0 && p[i].arrival_time <= time) {
                if (p[i].remaining_time < best_rt) {
                    best_rt = p[i].remaining_time;
                    idx = i;
                }
            }
        }
        if (idx == -1) {
            time++;
            last_pid = -1;
            continue;
        }
        if (p[idx].start_time == 0 && time >= p[idx].arrival_time) {
            p[idx].start_time = time;
        }

        if (last_pid != p[idx].pid) {
            timeline[tlen].time = time;
            timeline[tlen].pid = p[idx].pid;
            timeline[tlen].duration = 1;
            tlen++;
        } else {
            timeline[tlen-1].duration++;
        }

        last_pid = p[idx].pid;
        p[idx].remaining_time--;
        time++;

        if (p[idx].remaining_time == 0) {
            p[idx].completion_time = time;
        }
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (processes[j].pid == p[i].pid) {
                processes[j].start_time = p[i].start_time;
                processes[j].completion_time = p[i].completion_time;
                processes[j].remaining_time = 0;
            }
        }
    }

    *timeline_len = tlen;
    free(p);
}

/* ---------- Round Robin ---------- */

void schedule_rr(process_t *processes, int n, int quantum, timeline_event_t *timeline, int *timeline_len) {
    process_t *p = malloc(sizeof(process_t)*n);
    memcpy(p, processes, sizeof(process_t)*n);
    for (int i = 0; i < n; i++) p[i].remaining_time = p[i].burst_time;

    int time = 0;
    int tlen = 0;
    int remaining = n;

    while (remaining > 0) {
        int progressed = 0;
        for (int i = 0; i < n; i++) {
            if (p[i].remaining_time <= 0) continue;
            if (p[i].arrival_time > time) continue;

            if (p[i].start_time == 0 && time >= p[i].arrival_time)
                p[i].start_time = time;

            int run = (p[i].remaining_time > quantum) ? quantum : p[i].remaining_time;

            timeline[tlen].time = time;
            timeline[tlen].pid = p[i].pid;
            timeline[tlen].duration = run;
            tlen++;

            time += run;
            p[i].remaining_time -= run;
            progressed = 1;

            if (p[i].remaining_time == 0) {
                p[i].completion_time = time;
                remaining--;
            }
        }
        if (!progressed) {
            time++;
        }
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (processes[j].pid == p[i].pid) {
                processes[j].start_time = p[i].start_time;
                processes[j].completion_time = p[i].completion_time;
                processes[j].remaining_time = 0;
            }
        }
    }

    *timeline_len = tlen;
    free(p);
}

/* ---------- MLFQ (simplified implementation) ---------- */

void schedule_mlfq(process_t *processes, int n, mlfq_config_t *config, timeline_event_t *timeline, int *timeline_len) {
    int num_q = config->num_queues;
    int *quantum = config->quantums;
    int boost_interval = config->boost_interval;

    process_t *p = malloc(sizeof(process_t)*n);
    memcpy(p, processes, sizeof(process_t)*n);
    for (int i = 0; i < n; i++) p[i].remaining_time = p[i].burst_time;

    int time = 0;
    int tlen = 0;
    int remaining = n;

    int **queue = malloc(sizeof(int*) * num_q);
    int *head = calloc(num_q, sizeof(int));
    int *tail = calloc(num_q, sizeof(int));
    int maxN = n * 10;
    for (int i = 0; i < num_q; i++) {
        queue[i] = malloc(sizeof(int) * maxN);
    }

    int *in_queue = calloc(n, sizeof(int));

    while (remaining > 0) {
        for (int i = 0; i < n; i++) {
            if (!in_queue[i] && p[i].arrival_time <= time && p[i].remaining_time > 0) {
                queue[0][tail[0]++] = i;
                in_queue[i] = 1;
            }
        }

        if (boost_interval > 0 && time > 0 && time % boost_interval == 0) {
            for (int q = 1; q < num_q; q++) {
                while (head[q] < tail[q]) {
                    int idx = queue[q][head[q]++];
                    if (p[idx].remaining_time > 0) {
                        queue[0][tail[0]++] = idx;
                    }
                }
                head[q] = tail[q] = 0;
            }
        }

        int qsel = -1;
        for (int q = 0; q < num_q; q++) {
            if (head[q] < tail[q]) {
                qsel = q;
                break;
            }
        }
        if (qsel == -1) {
            time++;
            continue;
        }

        int idx = queue[qsel][head[qsel]++];
        in_queue[idx] = 0;
        if (p[idx].remaining_time <= 0) continue;

        if (p[idx].start_time == 0 && time >= p[idx].arrival_time)
            p[idx].start_time = time;

        int qtime = quantum[qsel];
        int run = (p[idx].remaining_time > qtime) ? qtime : p[idx].remaining_time;

        timeline[tlen].time = time;
        timeline[tlen].pid = p[idx].pid;
        timeline[tlen].duration = run;
        tlen++;

        time += run;
        p[idx].remaining_time -= run;

        if (p[idx].remaining_time == 0) {
            p[idx].completion_time = time;
            remaining--;
        } else {
            int new_q = qsel + 1;
            if (new_q >= num_q) new_q = num_q - 1;
            queue[new_q][tail[new_q]++] = idx;
            in_queue[idx] = 1;
        }
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (processes[j].pid == p[i].pid) {
                processes[j].start_time = p[i].start_time;
                processes[j].completion_time = p[i].completion_time;
                processes[j].remaining_time = 0;
            }
        }
    }

    *timeline_len = tlen;

    for (int i = 0; i < num_q; i++) free(queue[i]);
    free(queue);
    free(head);
    free(tail);
    free(in_queue);
    free(p);
}
