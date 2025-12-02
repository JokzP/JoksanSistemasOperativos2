#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <dirent.h>
#include <fcntl.h>

#include "procman.h"

process_info_t process_table[MAX_PROCESSES];
int process_count = 0;

/* mask used to block SIGCHLD while modifying the table */
static sigset_t sigchld_mask;

/* Forward declarations of internal helpers */
static int find_process_index(pid_t pid);
static void remove_terminated_from_table(void);
static void print_time_diff(time_t start);
static void install_signal_handlers(void);
static void interactive_shell(void);

/* ================= Utility functions ================= */

void init_process_table(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].pid = -1;
        process_table[i].status = -1;
        process_table[i].command[0] = '\0';
        process_table[i].start_time = 0;
        process_table[i].exit_code = 0;
    }
    process_count = 0;

    sigemptyset(&sigchld_mask);
    sigaddset(&sigchld_mask, SIGCHLD);
}

static int find_process_index(pid_t pid) {
    for (int i = 0; i < process_count; i++) {
        if (process_table[i].pid == pid) {
            return i;
        }
    }
    return -1;
}

static void add_process_to_table(pid_t pid, const char *cmd) {
    if (process_count >= MAX_PROCESSES) {
        fprintf(stderr, "Process table full, cannot track PID %d\n", pid);
        return;
    }
    process_table[process_count].pid = pid;
    strncpy(process_table[process_count].command, cmd, sizeof(process_table[process_count].command) - 1);
    process_table[process_count].command[sizeof(process_table[process_count].command) - 1] = '\0';
    process_table[process_count].start_time = time(NULL);
    process_table[process_count].status = 0;
    process_table[process_count].exit_code = 0;
    process_count++;
}

/* compact table by removing entries whose status != 0 */
static void remove_terminated_from_table(void) {
    int j = 0;
    for (int i = 0; i < process_count; i++) {
        if (process_table[i].status == 0) {
            if (i != j) {
                process_table[j] = process_table[i];
            }
            j++;
        }
    }
    process_count = j;
}

static void print_time_diff(time_t start) {
    if (start == 0) {
        printf("00:00:00");
        return;
    }
    time_t now = time(NULL);
    time_t diff = now - start;
    int hours = diff / 3600;
    int minutes = (diff % 3600) / 60;
    int seconds = diff % 60;
    printf("%02d:%02d:%02d", hours, minutes, seconds);
}

/* ================= Part 1: Basic process operations ================= */

int create_process(const char *command, char *args[]) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        fprintf(stderr, "Failed to create process: %s\n", strerror(errno));
        return -1;
    }

    if (pid == 0) {
        /* Child */
        execvp(command, args);
        perror("execvp");
        _exit(127);
    }

    /* Parent: store child in table */
    sigset_t oldmask;
    sigprocmask(SIG_BLOCK, &sigchld_mask, &oldmask);
    add_process_to_table(pid, command);
    sigprocmask(SIG_SETMASK, &oldmask, NULL);

    printf("Created process %d (%s)\n", pid, command);
    return pid;
}

int check_process_status(pid_t pid) {
    int status;
    pid_t result = waitpid(pid, &status, WNOHANG);
    if (result == 0) {
        /* still running */
        return 1;
    } else if (result == pid) {
        /* just reaped here */
        sigset_t oldmask;
        sigprocmask(SIG_BLOCK, &sigchld_mask, &oldmask);
        int idx = find_process_index(pid);
        if (idx >= 0) {
            process_table[idx].status = 1;
            if (WIFEXITED(status)) {
                process_table[idx].exit_code = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                process_table[idx].exit_code = 128 + WTERMSIG(status);
            }
        }
        sigprocmask(SIG_SETMASK, &oldmask, NULL);
        return 0;
    } else {
        if (result == -1 && errno == ECHILD) {
            /* already reaped, check our table */
            int idx = find_process_index(pid);
            if (idx >= 0 && process_table[idx].status != 0) {
                return 0; /* treated as terminated */
            }
        }
        perror("waitpid");
        return -1;
    }
}

int terminate_process(pid_t pid, int force) {
    int sig = force ? SIGKILL : SIGTERM;

    if (kill(pid, sig) == -1) {
        if (errno == ESRCH) {
            /* process does not exist anymore */
            fprintf(stderr, "Process %d does not exist\n", pid);
            return 0;
        }
        perror("kill");
        return -1;
    }

    int status;
    pid_t r = waitpid(pid, &status, 0);
    if (r == -1) {
        if (errno == ECHILD) {
            /* already reaped by SIGCHLD handler */
            return 0;
        }
        perror("waitpid");
        return -1;
    }

    sigset_t oldmask;
    sigprocmask(SIG_BLOCK, &sigchld_mask, &oldmask);
    int idx = find_process_index(pid);
    if (idx >= 0) {
        process_table[idx].status = 1;
        if (WIFEXITED(status)) {
            process_table[idx].exit_code = WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            process_table[idx].exit_code = 128 + WTERMSIG(status);
        }
    }
    sigprocmask(SIG_SETMASK, &oldmask, NULL);

    return 0;
}

/* ================= Part 2: Process manager operations ================= */

void list_processes(void) {
    printf("PID     COMMAND                 RUNTIME    STATUS\n");
    printf("-----   ---------------------   --------   ----------\n");

    for (int i = 0; i < process_count; i++) {
        const char *status_str = "Unknown";
        if (process_table[i].status == 0) {
            status_str = "Running";
        } else if (process_table[i].status == 1) {
            status_str = "Terminated";
        } else if (process_table[i].status == -1) {
            status_str = "Error";
        }

        printf("%-7d%-23s", process_table[i].pid, process_table[i].command);
        printf(" ");
        print_time_diff(process_table[i].start_time);
        printf("   %-10s\n", status_str);
    }
}

void wait_all_processes(void) {
    int remaining;
    do {
        remaining = 0;
        for (int i = 0; i < process_count; i++) {
            if (process_table[i].status == 0) {
                int status;
                pid_t r = waitpid(process_table[i].pid, &status, WNOHANG);
                if (r == 0) {
                    remaining++;
                } else if (r == process_table[i].pid) {
                    process_table[i].status = 1;
                    if (WIFEXITED(status)) {
                        process_table[i].exit_code = WEXITSTATUS(status);
                    } else if (WIFSIGNALED(status)) {
                        process_table[i].exit_code = 128 + WTERMSIG(status);
                    }
                } else if (r == -1 && errno != ECHILD) {
                    perror("waitpid");
                    process_table[i].status = -1;
                }
            }
        }
        if (remaining > 0) {
            /* sleep a bit before checking again */
            usleep(100000);
        }
    } while (remaining > 0);

    /* compact the table */
    sigset_t oldmask;
    sigprocmask(SIG_BLOCK, &sigchld_mask, &oldmask);
    remove_terminated_from_table();
    sigprocmask(SIG_SETMASK, &oldmask, NULL);

    printf("All child processes have completed.\n");
}

/* ================= Part 3: Signal handling ================= */

void sigint_handler(int signum) {
    (void)signum;
    const char msg[] = "Shutting down gracefully...\n";
    write(STDERR_FILENO, msg, sizeof(msg) - 1);

    /* Send SIGTERM to all running children */
    for (int i = 0; i < process_count; i++) {
        if (process_table[i].status == 0) {
            kill(process_table[i].pid, SIGTERM);
        }
    }

    /* Wait for all children to exit */
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, 0)) > 0) {
        (void)pid;
    }

    _exit(0);
}

void sigchld_handler(int signum) {
    (void)signum;
    int saved_errno = errno;
    int status;
    pid_t pid;

    /* Reap all terminated children */
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        /* Update process table */
        for (int i = 0; i < process_count; i++) {
            if (process_table[i].pid == pid) {
                process_table[i].status = 1;
                if (WIFEXITED(status)) {
                    process_table[i].exit_code = WEXITSTATUS(status);
                } else if (WIFSIGNALED(status)) {
                    process_table[i].exit_code = 128 + WTERMSIG(status);
                }
                break;
            }
        }
    }

    errno = saved_errno;
}

/* ================= Part 4: Process tree visualization ================= */

typedef struct {
    pid_t pid;
    pid_t ppid;
    char comm[256];
} proc_entry_t;

/* load process table from /proc */
static proc_entry_t *load_proc_entries(size_t *count) {
    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("opendir /proc");
        return NULL;
    }

    size_t cap = 64;
    size_t n = 0;
    proc_entry_t *arr = malloc(cap * sizeof(proc_entry_t));
    if (!arr) {
        closedir(dir);
        return NULL;
    }

    struct dirent *de;
    while ((de = readdir(dir)) != NULL) {
        char *endptr = NULL;
        long pid_long = strtol(de->d_name, &endptr, 10);
        if (*endptr != '\0') {
            continue; /* not a numeric directory */
        }
        if (pid_long <= 0) continue;
        pid_t pid = (pid_t)pid_long;

        char path[256];
        snprintf(path, sizeof(path), "/proc/%d/stat", pid);
        FILE *fp = fopen(path, "r");
        if (!fp) continue;

        int file_pid;
        char comm[256];
        char state;
        int ppid;
        if (fscanf(fp, "%d %255s %c %d", &file_pid, comm, &state, &ppid) != 4) {
            fclose(fp);
            continue;
        }
        fclose(fp);

        if (n >= cap) {
            cap *= 2;
            proc_entry_t *tmp = realloc(arr, cap * sizeof(proc_entry_t));
            if (!tmp) break;
            arr = tmp;
        }

        arr[n].pid = (pid_t)file_pid;
        arr[n].ppid = (pid_t)ppid;
        strncpy(arr[n].comm, comm, sizeof(arr[n].comm) - 1);
        arr[n].comm[sizeof(arr[n].comm) - 1] = '\0';
        n++;
    }

    closedir(dir);
    *count = n;
    return arr;
}

static void print_tree_recursive(proc_entry_t *entries, size_t n, pid_t root_pid, int depth, int is_last, int *stack_last) {
    for (int i = 0; i < depth; i++) {
        if (i == depth - 1) {
            printf("%s", stack_last[i] ? "└─ " : "├─ ");
        } else {
            printf("%s", stack_last[i] ? "   " : "│  ");
        }
    }

    printf("[%d]\n", root_pid);

    /* find children */
    size_t child_count = 0;
    for (size_t i = 0; i < n; i++) {
        if (entries[i].ppid == root_pid) child_count++;
    }

    size_t idx = 0;
    for (size_t i = 0; i < n; i++) {
        if (entries[i].ppid == root_pid) {
            int last_child = (idx == child_count - 1);
            stack_last[depth] = last_child;
            print_tree_recursive(entries, n, entries[i].pid, depth + 1, last_child, stack_last);
            idx++;
        }
    }
}

void print_process_tree(pid_t root_pid, int depth) {
    (void)depth; /* unused external parameter, tree depth is calculated inside */

    size_t n = 0;
    proc_entry_t *entries = load_proc_entries(&n);
    if (!entries) {
        fprintf(stderr, "Could not read /proc to build process tree\n");
        return;
    }

    int stack_last[64] = {0};

    /* print root with simple format: [pid] procman */
    printf("[%d] procman\n", root_pid);

    /* children of root */
    size_t child_count = 0;
    for (size_t i = 0; i < n; i++) {
        if (entries[i].ppid == root_pid) child_count++;
    }

    size_t idx = 0;
    for (size_t i = 0; i < n; i++) {
        if (entries[i].ppid == root_pid) {
            int last_child = (idx == child_count - 1);
            stack_last[0] = last_child;
            print_tree_recursive(entries, n, entries[i].pid, 1, last_child, stack_last);
            idx++;
        }
    }

    free(entries);
}

/* ================= Part 5: Interactive shell ================= */

static void install_signal_handlers(void) {
    struct sigaction sa_int;
    memset(&sa_int, 0, sizeof(sa_int));
    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);

    struct sigaction sa_chld;
    memset(&sa_chld, 0, sizeof(sa_chld));
    sa_chld.sa_handler = sigchld_handler;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa_chld, NULL);
}

static void print_help(void) {
    printf("Available commands:\n");
    printf("  help                          - Show this help\n");
    printf("  create <command> [args...]    - Create new process\n");
    printf("  list                          - List all processes\n");
    printf("  kill <pid> [force]            - Terminate process (force=1 uses SIGKILL)\n");
    printf("  tree                          - Show process tree\n");
    printf("  wait                          - Wait for all processes\n");
    printf("  quit                          - Exit program\n");
}

static void interactive_shell(void) {
    char line[1024];

    while (1) {
        printf("ProcMan> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        /* remove trailing newline */
        line[strcspn(line, "\n")] = '\0';

        if (strlen(line) == 0) {
            continue;
        }

        char *tokens[64];
        int ntokens = 0;
        char *saveptr = NULL;
        char *tok = strtok_r(line, " ", &saveptr);
        while (tok && ntokens < 63) {
            tokens[ntokens++] = tok;
            tok = strtok_r(NULL, " ", &saveptr);
        }
        tokens[ntokens] = NULL;
        if (ntokens == 0) continue;

        if (strcmp(tokens[0], "help") == 0) {
            print_help();
        } else if (strcmp(tokens[0], "create") == 0) {
            if (ntokens < 2) {
                fprintf(stderr, "Usage: create <command> [args...]\n");
                continue;
            }
            if (process_count >= MAX_PROCESSES) {
                fprintf(stderr, "Process table full, cannot create more processes.\n");
                continue;
            }

            char *cmd = tokens[1];
            char *args[64];
            int i;
            for (i = 1; i < ntokens && i < 63; i++) {
                args[i-1] = tokens[i];
            }
            args[i-1] = NULL;

            create_process(cmd, args);

        } else if (strcmp(tokens[0], "list") == 0) {
            list_processes();
        } else if (strcmp(tokens[0], "kill") == 0) {
            if (ntokens < 2) {
                fprintf(stderr, "Usage: kill <pid> [force]\n");
                continue;
            }
            pid_t pid = (pid_t)atoi(tokens[1]);
            int force = 0;
            if (ntokens >= 3) {
                force = atoi(tokens[2]) != 0;
            }
            terminate_process(pid, force);
        } else if (strcmp(tokens[0], "tree") == 0) {
            pid_t root = getpid();
            print_process_tree(root, 0);
        } else if (strcmp(tokens[0], "wait") == 0) {
            wait_all_processes();
        } else if (strcmp(tokens[0], "quit") == 0 || strcmp(tokens[0], "exit") == 0) {
            printf("Shutting down...\n");
            break;
        } else {
            fprintf(stderr, "Unknown command: %s\n", tokens[0]);
            print_help();
        }
    }
}

int main(void) {
    init_process_table();
    install_signal_handlers();
    print_help();
    interactive_shell();
    /* Before exit, try to terminate any remaining children gracefully */
    for (int i = 0; i < process_count; i++) {
        if (process_table[i].status == 0) {
            kill(process_table[i].pid, SIGTERM);
        }
    }
    return 0;
}
