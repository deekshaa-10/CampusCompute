#ifndef JOBS_H
#define JOBS_H
#include "job.h"
int add_job(Job jobs[], int *count, int id, int arrival, int burst, int priority);
void show_jobs(const Job jobs[], int count);
void mark_jobs_completed(Job jobs[], int count);
#endif
