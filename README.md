# CampusCompute: Intelligent Job Scheduling and Resource Management

**Operating Systems (UE25CS242A) - Level 3 Mini Project**

## Team Members
- Aditi Saraf (PES1UG25CS031): producer-consumer logger, shared-memory IPC, worker/process management (fork/waitpid)
- Deeksha A (PES1UG26CS813): CPU scheduling (FCFS, RR, Priority), Banker's safety algorithm, resource limits, job management

## Requirements
Ubuntu Linux or WSL Ubuntu; GCC and Make (`sudo apt install build-essential`).

## Build
`make clean && make`

## Run
`./campuscompute` for the interactive menu.
`./campuscompute help` for all command options.
`./campuscompute all` for an integrated demonstration.
Individual tests: `./campuscompute fcfs`, `rr`, `priority`, `compare`, `banker-safe`, `banker-unsafe`, `logger`, `ipc`, `worker`, `limits`.

The menu allows custom jobs (ID, arrival, burst, priority), a configurable positive RR quantum, and job status display. Scheduling is a deterministic simulation; statuses become completed when a scheduling demonstration finishes. It does not run user-submitted CPU workloads.

Priority uses smaller numbers as higher priority. FCFS ties use job ID. The scheduler's tie-break behaviour is in `scheduler.c`.

## Cleanup
`make clean` removes compiled objects and the executable. The logger writes `logs/activity.log`; remove this file manually if desired (`rm -f logs/activity.log`). IPC demos unlink their own shared memory and named semaphores. The process demos reap child processes and the logger joins threads.

## Notes
The Banker examples demonstrate safe and unsafe states. Worker failure is explicitly simulated (exit code 7). The resource-limit demo demonstrates RLIMIT_NOFILE in a child process.
