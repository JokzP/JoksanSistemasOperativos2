# ALGORITHMS.md - Scheduling Algorithms

## FIFO (First In First Out)

- Non-preemptive.
- Processes are ordered by arrival time.
- Each process runs to completion once started.
- Simple, but can have poor waiting time for jobs that arrive later (convoy effect).

## SJF (Shortest Job First)

- Non-preemptive.
- Among all arrived processes, pick the one with the smallest burst time.
- Minimizes average waiting time for known burst times.
- Can starve long jobs if short jobs keep arriving.

## STCF (Shortest Time to Completion First)

- Preemptive version of SJF.
- At each time step, scheduler chooses the process with smallest remaining time.
- Can preempt running process when a new process arrives with shorter remaining time.
- Optimal for minimizing average turnaround time.

## Round Robin

- Preemptive with fixed time quantum `q`.
- Scheduler cycles through processes in arrival order.
- A process runs for `min(q, remaining_time)` then goes to the back of the queue.
- Good for interactive systems and fairness.
- Quantum too large → approximates FIFO. Quantum too small → overhead.

## MLFQ (Multi-Level Feedback Queue)

- Multiple priority queues, each with its own time quantum.
- New processes start at highest priority queue.
- If a process uses its full quantum, it is demoted to a lower-priority queue.
- If it yields or finishes early, it may stay in the same queue.
- Periodic priority boost moves all processes back to top queue.
- Balances interactive (short/IO-bound) and CPU-bound jobs.
