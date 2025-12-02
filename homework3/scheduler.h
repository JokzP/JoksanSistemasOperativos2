#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <time.h>

#define MAX_PROCESSES 64
#define MAX_TIMELINE_EVENTS 1024

typedef struct {
    int pid;                    /* Process ID */
    int arrival_time;           /* When process arrives */
    int burst_time;             /* Total CPU time needed */
    int priority;               /* Priority (lower = higher priority) */
    int remaining_time;         /* Time left to execute */
    int start_time;             /* First time scheduled */
    int completion_time;        /* When finished */
    int turnaround_time;        /* completion - arrival */
    int waiting_time;           /* turnaround - burst */
    int response_time;          /* start - arrival */
} process_t;

typedef struct {
    int time;                   /* Time slice start */
    int pid;                    /* Process running */
    int duration;               /* How long it ran */
} timeline_event_t;

#endif /* SCHEDULER_H */
