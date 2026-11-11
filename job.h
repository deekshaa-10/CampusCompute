#ifndef JOB_H
#define JOB_H

#define MAX_JOBS 32

typedef struct {
    int id;
    int arrival_time;
    int burst_time;
    int priority;       /* Lower number means higher priority. */
    int status;         /* 0 = waiting, 1 = running, 2 = completed. */
} Job;

typedef struct {
    int job_id;         /* -1 represents CPU idle time. */
    int start_time;
    int end_time;
} ScheduleSegment;

typedef struct {
    int job_id;
    int completion_time;
    int turnaround_time;
    int waiting_time;
} JobMetrics;

#endif
