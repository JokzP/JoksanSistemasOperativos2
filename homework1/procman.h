#ifndef PROCMAN_H
#define PROCMAN_H

#include <sys/types.h>
#include <time.h>

#define MAX_PROCESSES 10

typedef struct {
    pid_t pid;
    char command[256];
    time_t start_time;
    int status;      /* 0 = running, 1 = terminated, -1 = error */
    int exit_code;   /* exit status if terminated */
} process_info_t;

extern process_info_t process_table[MAX_PROCESSES];
extern int process_count;

/* Part 1: Basic process operations */
int create_process(const char *command, char *args[]);
int check_process_status(pid_t pid);
int terminate_process(pid_t pid, int force);

/* Part 2: Process manager operations */
void list_processes(void);
void wait_all_processes(void);

/* Part 3: Signal handling */
void sigint_handler(int signum);
void sigchld_handler(int signum);

/* Part 4: Process tree visualization */
void print_process_tree(pid_t root_pid, int depth);

/* Utility */
void init_process_table(void);

#endif /* PROCMAN_H */
