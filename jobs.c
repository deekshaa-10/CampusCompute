#include <stdio.h>
#include "jobs.h"
int add_job(Job jobs[], int *count, int id, int arrival, int burst, int priority) {
    if (*count >= MAX_JOBS || id < 0 || arrival < 0 || burst <= 0 || priority < 0) {
        puts("Invalid job: check capacity, ID, arrival, burst (>0), and priority."); return 1;
    }
    for (int i=0;i<*count;i++) if (jobs[i].id==id) { puts("Duplicate job ID."); return 1; }
    jobs[*count]=(Job){id,arrival,burst,priority,0}; ++*count;
    printf("Added J%d (waiting).\n",id); return 0;
}
void show_jobs(const Job jobs[], int count) {
    puts("ID  Arrival  Burst  Priority  Status");
    for (int i=0;i<count;i++) printf("%-3d %-8d %-6d %-9d %s\n",jobs[i].id,jobs[i].arrival_time,jobs[i].burst_time,jobs[i].priority,jobs[i].status==2?"completed":jobs[i].status==1?"running":"waiting");
    if (!count) puts("No jobs. Add a job first.");
}
void mark_jobs_completed(Job jobs[], int count) {for(int i=0;i<count;i++)jobs[i].status=2;}
