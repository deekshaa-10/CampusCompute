# CampusCompute

A command-line prototype for the OS mini-project. It simulates job scheduling,
concurrent activity logging, inter-process communication, the Banker's safety
algorithm, worker-failure detection, and process resource limits, written in C
on Linux.

## Requirements
- Ubuntu Linux or WSL Ubuntu
- GCC and Make (`sudo apt install build-essential`)

## Build
```
make clean && make
```
This produces the `campuscompute` executable.

## Running it
Start the interactive menu:
```
./campuscompute
```
Or run a single feature directly:
```
./campuscompute fcfs
./campuscompute rr
./campuscompute priority
./campuscompute compare
./campuscompute banker-safe
./campuscompute banker-unsafe
./campuscompute logger
./campuscompute ipc
./campuscompute worker
./campuscompute limits
./campuscompute all      # runs everything in sequence
./campuscompute help     # lists all commands
```
From the menu you can add your own jobs (ID, arrival, burst, priority), set the
Round Robin quantum, and show job status.

## Notes on behaviour
- Scheduling is a deterministic simulation. A job is marked completed once a
  scheduling run finishes; the program does not execute real user workloads.
- Priority scheduling treats smaller numbers as higher priority. Ties go to
  earlier arrival, then smaller job ID. The logic is in `scheduler.c`.
- Worker failure is simulated on purpose (the child exits with status 7), so it
  is not a real crash.
- The resource-limit demo applies RLIMIT_NOFILE inside a child process and shows
  `open()` failing once the limit is reached.

## Cleanup
- `make clean` removes the object files and the executable.
- The logger writes `logs/activity.log`. Remove it with `rm -f logs/activity.log`
  if you want.
- The IPC demo unlinks its own shared memory and named semaphore, the process
  demos reap their children, and the logger joins its threads, so nothing is left
  behind.
