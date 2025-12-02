# DESIGN.md - CPU Scheduler Simulator

## Data Structures

- `process_t` stores all per-process scheduling fields (arrival, burst, priority, remaining, start, completion, turnaround, waiting, response).
- `timeline_event_t` tracks when each PID runs and for how long, used for Gantt chart visualizations.

## Modules

- `algorithms.c` implements five algorithms:
  - FIFO (FCFS)
  - SJF (non-preemptive)
  - STCF (preemptive SJF)
  - Round Robin
  - MLFQ (simplified, multi-queue with quantums and periodic boost)
- `metrics.c` calculates:
  - average turnaround, waiting, response time
  - CPU utilization
  - throughput
  - Jain's fairness index
- `scheduler.c` is a CLI entry point that:
  - loads workloads from text files
  - runs algorithms
  - generates a Markdown report via `report.c`
- `gui_ncurses.c` is a simple stubbed text UI (no full ncurses visual yet).
- `gui_gtk.c` is a placeholder to compile if GTK GUI is implemented later.

## Design Choices

- Algorithms work on copies of the process array to preserve original workload.
- Time is simulated as discrete integer units.
- MLFQ uses simple round-robin per level and demotion when a process uses its full quantum.
- Boosting: every `boost_interval` units, all processes are moved back to the top queue.
