#ifndef BANKER_H
#define BANKER_H

#define BANKER_MAX_PROCESSES 10
#define BANKER_MAX_RESOURCES 10

/*
 * Returns 1 if the state is safe, 0 if unsafe or invalid.
 * need is calculated internally as maximum - allocation.
 * A safe sequence is printed when one exists.
 */
int banker_check_safety(
    int process_count,
    int resource_count,
    const int available[],
    const int allocation[BANKER_MAX_PROCESSES][BANKER_MAX_RESOURCES],
    const int maximum[BANKER_MAX_PROCESSES][BANKER_MAX_RESOURCES]
);

#endif
