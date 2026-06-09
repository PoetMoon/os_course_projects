#include "workload.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int rand_range(int low, int high) {
    return low + rand() % (high - low + 1);
}

static void setup_process(Process *p, int pid, int arrival, int burst, int priority, const char *type) {
    init_process(p);
    p->pid = pid;
    p->arrival_time = arrival;
    p->burst_time = burst;
    p->remaining_time = burst;
    p->priority = priority;
    p->total_run_time = 0;
    p->last_ready_time = arrival;
    p->queue_level = 0;
    p->aging_score = 0.0;
    strncpy(p->process_type, type, sizeof(p->process_type) - 1);
}

int generate_workload(
    const char *scenario,
    int n,
    int seed,
    Process processes[]
) {
    if (n <= 0) {
        return 0;
    }
    if (n > MAX_PROCESSES) {
        n = MAX_PROCESSES;
    }

    srand((unsigned int)seed);

    if (strcmp(scenario, "short_jobs") == 0) {
        for (int i = 0; i < n; ++i) {
            setup_process(
                &processes[i],
                i + 1,
                rand_range(0, 20),
                rand_range(1, 6),
                rand_range(1, 5),
                "short"
            );
        }
    } else if (strcmp(scenario, "long_jobs") == 0) {
        for (int i = 0; i < n; ++i) {
            setup_process(
                &processes[i],
                i + 1,
                rand_range(0, 20),
                rand_range(20, 60),
                rand_range(1, 5),
                "long"
            );
        }
    } else if (strcmp(scenario, "mixed") == 0) {
        for (int i = 0; i < n; ++i) {
            int burst = 0;
            const char *type = "mixed_medium";
            int bucket = i % 3;
            if (bucket == 0) {
                burst = rand_range(1, 6);
                type = "mixed_short";
            } else if (bucket == 1) {
                burst = rand_range(7, 20);
                type = "mixed_medium";
            } else {
                burst = rand_range(21, 60);
                type = "mixed_long";
            }
            setup_process(
                &processes[i],
                i + 1,
                rand_range(0, 50),
                burst,
                rand_range(1, 5),
                type
            );
        }
    } else if (strcmp(scenario, "interactive") == 0) {
        for (int i = 0; i < n; ++i) {
            setup_process(
                &processes[i],
                i + 1,
                rand_range(0, 40) + i / 3,
                rand_range(1, 10),
                rand_range(1, 3),
                "interactive"
            );
        }
    } else if (strcmp(scenario, "starvation_case") == 0) {
        int early_long = n < 4 ? n : 4;
        for (int i = 0; i < early_long; ++i) {
            setup_process(
                &processes[i],
                i + 1,
                i,
                rand_range(40, 60),
                5,
                "long_starvation"
            );
        }
        for (int i = early_long; i < n; ++i) {
            setup_process(
                &processes[i],
                i + 1,
                4 + (i - early_long),
                rand_range(1, 4),
                1,
                "short_flood"
            );
        }
    } else if (strcmp(scenario, "behavior_sensitive") == 0) {
        int batch_count = n / 5;
        int latency_count = n - batch_count;

        if (batch_count < 2 && n >= 6) {
            batch_count = 2;
            latency_count = n - batch_count;
        }

        for (int i = 0; i < batch_count; ++i) {
            setup_process(
                &processes[i],
                i + 1,
                rand_range(0, 3),
                rand_range(45, 80),
                5,
                "cpu_bound_batch"
            );
        }

        for (int i = 0; i < latency_count; ++i) {
            int idx = batch_count + i;
            setup_process(
                &processes[idx],
                idx + 1,
                2 + i * 2 + rand_range(0, 2),
                rand_range(1, 4),
                1,
                "latency_sensitive"
            );
        }
    } else {
        fprintf(stderr, "Unknown scenario: %s\n", scenario);
        return 0;
    }

    return n;
}
