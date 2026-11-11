#include <stdio.h>
#include "banker.h"

int banker_check_safety(
    int process_count,
    int resource_count,
    const int available[],
    const int allocation[BANKER_MAX_PROCESSES][BANKER_MAX_RESOURCES],
    const int maximum[BANKER_MAX_PROCESSES][BANKER_MAX_RESOURCES])
{
    if (process_count < 1 || process_count > BANKER_MAX_PROCESSES ||
        resource_count < 1 || resource_count > BANKER_MAX_RESOURCES ||
        !available || !allocation || !maximum) {
        fprintf(stderr, "Banker's algorithm: invalid dimensions or input.\n");
        return 0;
    }

    int work[BANKER_MAX_RESOURCES];
    int need[BANKER_MAX_PROCESSES][BANKER_MAX_RESOURCES] = {{0}};
    int finish[BANKER_MAX_PROCESSES] = {0};
    int safe_sequence[BANKER_MAX_PROCESSES];

    for (int r = 0; r < resource_count; ++r) {
        if (available[r] < 0) {
            fprintf(stderr, "Banker's algorithm: available resources cannot be negative.\n");
            return 0;
        }
        work[r] = available[r];
    }

    for (int p = 0; p < process_count; ++p) {
        for (int r = 0; r < resource_count; ++r) {
            if (allocation[p][r] < 0 || maximum[p][r] < 0 ||
                maximum[p][r] < allocation[p][r]) {
                fprintf(stderr, "Banker's algorithm: invalid allocation/maximum at P%d, R%d.\n",
                        p, r);
                return 0;
            }
            need[p][r] = maximum[p][r] - allocation[p][r];
        }
    }

    printf("\nNeed matrix (Maximum - Allocation):\n");
    for (int p = 0; p < process_count; ++p) {
        printf("P%d: ", p);
        for (int r = 0; r < resource_count; ++r)
            printf("%d%s", need[p][r], r + 1 == resource_count ? "\n" : " ");
    }

    int count = 0;
    while (count < process_count) {
        int found = 0;
        for (int p = 0; p < process_count; ++p) {
            if (finish[p]) continue;

            int can_finish = 1;
            for (int r = 0; r < resource_count; ++r) {
                if (need[p][r] > work[r]) {
                    can_finish = 0;
                    break;
                }
            }

            if (can_finish) {
                for (int r = 0; r < resource_count; ++r)
                    work[r] += allocation[p][r];
                safe_sequence[count++] = p;
                finish[p] = 1;
                found = 1;
            }
        }
        if (!found) break;
    }

    if (count == process_count) {
        printf("SAFE state. Safe sequence: ");
        for (int i = 0; i < process_count; ++i)
            printf("P%d%s", safe_sequence[i],
                   i + 1 == process_count ? "\n" : " -> ");
        return 1;
    }

    printf("UNSAFE state. No safe sequence exists for the supplied state.\n");
    return 0;
}
