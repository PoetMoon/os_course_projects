#ifndef METRICS_H
#define METRICS_H

#include "process.h"

typedef struct {
    char algorithm[32];
    char scenario[32];

    double average_turnaround_time;
    double average_weighted_turnaround_time;
    double average_waiting_time;
    double average_response_time;

    int max_waiting_time;
    double waiting_time_variance;

    int context_switch_count;
    double cpu_utilization;
    double throughput;

    int starvation_count;
    double starvation_rate;
    double fairness_index;
} Metrics;

void compute_metrics(
    Metrics *metrics,
    const char *algorithm,
    const char *scenario,
    Process processes[],
    int n,
    int context_switch_count,
    int total_time,
    int busy_time
);

#endif
