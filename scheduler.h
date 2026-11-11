#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "job.h"

/* All functions leave the caller's input array unchanged. */
void run_fcfs(const Job jobs[], int count);
void run_round_robin(const Job jobs[], int count, int quantum);
void run_priority_non_preemptive(const Job jobs[], int count);
void compare_schedulers(const Job jobs[], int count, int quantum);

#endif
