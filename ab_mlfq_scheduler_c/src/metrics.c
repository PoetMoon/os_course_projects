#include "metrics.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

void compute_metrics(
    Metrics *metrics,
    const char *algorithm,
    const char *scenario,
    Process processes[],
    int n,
    int context_switch_count,
    int total_time,
    int busy_time
) {
    double turnaround_sum = 0.0;
    double weighted_sum = 0.0;
    double waiting_sum = 0.0;
    double response_sum = 0.0;
    double waiting_sq_sum = 0.0;
    int max_waiting = 0;
    int starvation_count = 0;
    int completed_count = 0;

    memset(metrics, 0, sizeof(*metrics));
    strncpy(metrics->algorithm, algorithm, sizeof(metrics->algorithm) - 1);
    strncpy(metrics->scenario, scenario, sizeof(metrics->scenario) - 1);

    for (int i = 0; i < n; ++i) {
        if (processes[i].finish_time >= 0) {
            processes[i].turnaround_time = processes[i].finish_time - processes[i].arrival_time;
            processes[i].waiting_time = processes[i].turnaround_time - processes[i].burst_time;
            completed_count++;
        } else {
            processes[i].turnaround_time = 0;
            processes[i].waiting_time = 0;
        }

        turnaround_sum += processes[i].turnaround_time;
        waiting_sum += processes[i].waiting_time;
        waiting_sq_sum += (double)processes[i].waiting_time * (double)processes[i].waiting_time;
        if (processes[i].burst_time > 0) {
            weighted_sum += (double)processes[i].turnaround_time / (double)processes[i].burst_time;
        }
        if (processes[i].response_time >= 0) {
            response_sum += processes[i].response_time;
        }
        if (processes[i].waiting_time > max_waiting) {
            max_waiting = processes[i].waiting_time;
        }
        if (processes[i].max_waiting_streak >= 30) {
            starvation_count++;
        }
    }

    metrics->average_turnaround_time = n > 0 ? turnaround_sum / n : 0.0;
    metrics->average_weighted_turnaround_time = n > 0 ? weighted_sum / n : 0.0;
    metrics->average_waiting_time = n > 0 ? waiting_sum / n : 0.0;
    metrics->average_response_time = n > 0 ? response_sum / n : 0.0;
    metrics->max_waiting_time = max_waiting;
    metrics->waiting_time_variance =
        n > 0 ? (waiting_sq_sum / n) - (metrics->average_waiting_time * metrics->average_waiting_time) : 0.0;
    if (metrics->waiting_time_variance < 0.0) {
        metrics->waiting_time_variance = 0.0;
    }

    metrics->context_switch_count = context_switch_count;
    metrics->cpu_utilization = total_time > 0 ? (double)busy_time / (double)total_time : 0.0;
    metrics->throughput = total_time > 0 ? (double)completed_count / (double)total_time : 0.0;
    metrics->starvation_count = starvation_count;
    metrics->starvation_rate = n > 0 ? (double)starvation_count / (double)n : 0.0;
    metrics->fairness_index = 1.0 / (1.0 + metrics->waiting_time_variance);
}
