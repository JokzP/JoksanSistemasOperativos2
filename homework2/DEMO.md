# DEMO - Sample Outputs

## Test 1: Create and Verify Zombies

```bash
$ ./zombie_creator 3
Created zombie: PID 1234 (exit code 0)
Created zombie: PID 1235 (exit code 1)
Created zombie: PID 1236 (exit code 2)
Press Enter to exit and clean up zombies...
```

En otra terminal:

```bash
$ ps aux | grep 'Z'
user   1234  0.0  0.0  0  0 ?  Z    00:00   0:00 [zombie_creator] <defunct>
...
```

## Test 2: Zombie Detector

```bash
$ ./zombie_detector
=== Zombie Process Report ===
Total Zombies: 3

PID     PPID    Command         State   Time
-----   -----   -------------   -----   --------
1234    1000    defunct         Z       00:00:00
1235    1000    defunct         Z       00:00:00
1236    1000    defunct         Z       00:00:00

Parent Process Analysis:
  PID 1000 has 3 zombie children
```

## Test 3: Reaping Strategies

```bash
$ ./zombie_reaper 1
# wait() reaps all children explicitly
# ps output should show no defunct processes

$ ./zombie_reaper 2
# SIGCHLD handler reaps children automatically

$ ./zombie_reaper 3
# SIGCHLD is ignored, kernel auto-reaps children
```

## Test 4: Daemon

```bash
$ ./process_daemon
$ ps aux | grep process_daemon
user  2345  0.0  0.0  ...  process_daemon

$ watch -n 1 'ps aux | grep defunct'
# No zombies should appear while daemon is running

$ killall process_daemon
```

## Test 5: Library Example

```bash
$ ./test_lib
Spawned child PID 3456
Spawned child PID 3457
Spawned child PID 3458
Waiting for children to exit...
Zombie stats:
  Created: 3
  Reaped : 3
  Active : 0
```
