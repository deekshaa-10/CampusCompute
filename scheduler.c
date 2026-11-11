#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"

static int total_burst(const Job jobs[], int count) {
    int total = 0;
    for (int i = 0; i < count; ++i) total += jobs[i].burst_time;
    return total;
}

static void print_results(const char *title, const JobMetrics metrics[], int count) {
    double total_tat = 0.0, total_wt = 0.0;
    printf("\n%s\n", title);
    printf("%-8s %-12s %-14s %-12s\n",
           "Job", "Completion", "Turnaround", "Waiting");
    for (int i = 0; i < count; ++i) {
        printf("J%-7d %-12d %-14d %-12d\n",
               metrics[i].job_id, metrics[i].completion_time,
               metrics[i].turnaround_time, metrics[i].waiting_time);
        total_tat += metrics[i].turnaround_time;
        total_wt += metrics[i].waiting_time;
    }
    printf("Average turnaround time: %.2f\n", total_tat / count);
    printf("Average waiting time:    %.2f\n", total_wt / count);
}

static void print_timeline(const ScheduleSegment segments[], int used) {
    printf("\nExecution timeline:\n");
    for (int i = 0; i < used; ++i) {
        if (segments[i].job_id == -1)
            printf("| IDLE (%d-%d) ", segments[i].start_time, segments[i].end_time);
        else
            printf("| J%d (%d-%d) ", segments[i].job_id,
                   segments[i].start_time, segments[i].end_time);
    }
    printf("|\n");
}

static void initialize_metrics(const Job jobs[], JobMetrics metrics[], int count) {
    for (int i = 0; i < count; ++i) {
        metrics[i].job_id = jobs[i].id;
        metrics[i].completion_time = 0;
        metrics[i].turnaround_time = 0;
        metrics[i].waiting_time = 0;
    }
}

static int find_original_index(const Job jobs[], int count, int id) {
    for (int i = 0; i < count; ++i)
        if (jobs[i].id == id) return i;
    return -1;
}

static int validate_jobs(const Job jobs[], int count) {
    if (!jobs || count < 1 || count > MAX_JOBS) {
        fprintf(stderr, "Error: job count must be between 1 and %d.\n", MAX_JOBS);
        return 0;
    }
    for (int i = 0; i < count; ++i) {
        if (jobs[i].id < 0 || jobs[i].arrival_time < 0 || jobs[i].burst_time <= 0) {
            fprintf(stderr, "Error: job IDs/arrival times must be non-negative "
                            "and burst times must be positive.\n");
            return 0;
        }
        for (int j = i + 1; j < count; ++j) {
            if (jobs[i].id == jobs[j].id) {
                fprintf(stderr, "Error: job IDs must be unique.\n");
                return 0;
            }
        }
    }
    return 1;
}

static int earlier_arrival(const Job *a, const Job *b) {
    if (a->arrival_time != b->arrival_time)
        return a->arrival_time < b->arrival_time;
    return a->id < b->id;
}

void run_fcfs(const Job jobs[], int count) {
    if (!validate_jobs(jobs, count)) return;

    Job ordered[MAX_JOBS];
    JobMetrics metrics[MAX_JOBS];
    ScheduleSegment *segments = calloc((size_t)count + 1, sizeof(*segments));
    if (!segments) { perror("calloc"); return; }

    memcpy(ordered, jobs, (size_t)count * sizeof(Job));
    initialize_metrics(jobs, metrics, count);

    /* Insertion sort: arrival time, then smaller job ID. */
    for (int i = 1; i < count; ++i) {
        Job key = ordered[i];
        int j = i - 1;
        while (j >= 0 && earlier_arrival(&key, &ordered[j])) {
            ordered[j + 1] = ordered[j];
            --j;
        }
        ordered[j + 1] = key;
    }

    int time = 0, used = 0;
    for (int i = 0; i < count; ++i) {
        if (time < ordered[i].arrival_time) {
            segments[used++] = (ScheduleSegment){-1, time, ordered[i].arrival_time};
            time = ordered[i].arrival_time;
        }
        int start = time;
        time += ordered[i].burst_time;
        segments[used++] = (ScheduleSegment){ordered[i].id, start, time};

        int original = find_original_index(jobs, count, ordered[i].id);
        metrics[original].completion_time = time;
        metrics[original].turnaround_time = time - ordered[i].arrival_time;
        metrics[original].waiting_time =
            metrics[original].turnaround_time - ordered[i].burst_time;
    }

    print_timeline(segments, used);
    print_results("FCFS results (tie-break: earlier arrival, then smaller job ID)",
                  metrics, count);
    free(segments);
}

void run_priority_non_preemptive(const Job jobs[], int count) {
    if (!validate_jobs(jobs, count)) return;

    JobMetrics metrics[MAX_JOBS];
    int completed[MAX_JOBS] = {0};
    int capacity = count + 1, used = 0, time = 0, finished = 0;
    ScheduleSegment *segments = calloc((size_t)capacity, sizeof(*segments));
    if (!segments) { perror("calloc"); return; }
    initialize_metrics(jobs, metrics, count);

    while (finished < count) {
        int best = -1;
        for (int i = 0; i < count; ++i) {
            if (completed[i] || jobs[i].arrival_time > time) continue;
            if (best == -1 ||
                jobs[i].priority < jobs[best].priority ||
                (jobs[i].priority == jobs[best].priority &&
                 (jobs[i].arrival_time < jobs[best].arrival_time ||
                  (jobs[i].arrival_time == jobs[best].arrival_time &&
                   jobs[i].id < jobs[best].id)))) {
                best = i;
            }
        }

        if (best == -1) {
            int next_arrival = -1;
            for (int i = 0; i < count; ++i)
                if (!completed[i] &&
                    (next_arrival == -1 || jobs[i].arrival_time < next_arrival))
                    next_arrival = jobs[i].arrival_time;
            segments[used++] = (ScheduleSegment){-1, time, next_arrival};
            time = next_arrival;
            continue;
        }

        int start = time;
        time += jobs[best].burst_time;
        segments[used++] = (ScheduleSegment){jobs[best].id, start, time};
        completed[best] = 1;
        ++finished;

        metrics[best].completion_time = time;
        metrics[best].turnaround_time = time - jobs[best].arrival_time;
        metrics[best].waiting_time =
            metrics[best].turnaround_time - jobs[best].burst_time;
    }

    print_timeline(segments, used);
    print_results("Non-preemptive Priority results (smaller number = higher priority; "
                  "ties: arrival, then job ID)", metrics, count);
    free(segments);
}

void run_round_robin(const Job jobs[], int count, int quantum) {
    if (!validate_jobs(jobs, count)) return;
    if (quantum <= 0) {
        fprintf(stderr, "Error: Round Robin quantum must be positive.\n");
        return;
    }

    int remaining[MAX_JOBS];
    int total = total_burst(jobs, count);
    int segment_capacity = total + count + 2;
    ScheduleSegment *segments =
        calloc((size_t)segment_capacity, sizeof(*segments));
    JobMetrics metrics[MAX_JOBS];
    int queue[MAX_JOBS + 1], head = 0, tail = 0, queued = 0;
    int time = 0, finished = 0, used = 0, next_to_arrive = 0;
    int arrival_order[MAX_JOBS];

    if (!segments) { perror("calloc"); return; }
    initialize_metrics(jobs, metrics, count);
    for (int i = 0; i < count; ++i) {
        remaining[i] = jobs[i].burst_time;
        arrival_order[i] = i;
    }

    /* Sort indices by arrival time, then job ID. */
    for (int i = 1; i < count; ++i) {
        int key = arrival_order[i], j = i - 1;
        while (j >= 0 && earlier_arrival(&jobs[key], &jobs[arrival_order[j]])) {
            arrival_order[j + 1] = arrival_order[j];
            --j;
        }
        arrival_order[j + 1] = key;
    }

    while (finished < count) {
        while (next_to_arrive < count &&
               jobs[arrival_order[next_to_arrive]].arrival_time <= time) {
            int idx = arrival_order[next_to_arrive++];
            queue[tail] = idx;
            tail = (tail + 1) % (MAX_JOBS + 1);
            ++queued;
        }

        if (queued == 0) {
            int next_time = jobs[arrival_order[next_to_arrive]].arrival_time;
            segments[used++] = (ScheduleSegment){-1, time, next_time};
            time = next_time;
            continue;
        }

        int idx = queue[head];
        head = (head + 1) % (MAX_JOBS + 1);
        --queued;

        int run_for = remaining[idx] < quantum ? remaining[idx] : quantum;
        int start = time;
        time += run_for;
        remaining[idx] -= run_for;
        segments[used++] = (ScheduleSegment){jobs[idx].id, start, time};

        /* Add jobs that arrived during this time slice before re-queuing. */
        while (next_to_arrive < count &&
               jobs[arrival_order[next_to_arrive]].arrival_time <= time) {
            int new_idx = arrival_order[next_to_arrive++];
            queue[tail] = new_idx;
            tail = (tail + 1) % (MAX_JOBS + 1);
            ++queued;
        }

        if (remaining[idx] > 0) {
            queue[tail] = idx;
            tail = (tail + 1) % (MAX_JOBS + 1);
            ++queued;
        } else {
            ++finished;
            metrics[idx].completion_time = time;
            metrics[idx].turnaround_time = time - jobs[idx].arrival_time;
            metrics[idx].waiting_time =
                metrics[idx].turnaround_time - jobs[idx].burst_time;
        }
    }

    printf("\nRound Robin time quantum: %d\n", quantum);
    print_timeline(segments, used);
    print_results("Round Robin results", metrics, count);
    free(segments);
}

void compare_schedulers(const Job jobs[], int count, int quantum) {
    printf("\n========== FCFS ==========\n");
    run_fcfs(jobs, count);
    printf("\n========== ROUND ROBIN ==========\n");
    run_round_robin(jobs, count, quantum);
    printf("\n========== PRIORITY ==========\n");
    run_priority_non_preemptive(jobs, count);
}
