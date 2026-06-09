#include "scheduler.h"

#include <stdlib.h>
#include <string.h>

static void record_timeline(TimelineEntry timeline[], int *count, int time, int pid) {
    if (*count >= MAX_TIMELINE) {
        return;
    }
    timeline[*count].time = time;
    timeline[*count].pid = pid;
    strcpy(timeline[*count].algorithm, "fcfs");
    timeline[*count].queue = -1;
    (*count)++;
}

static void update_waiting(Process processes[], int n, int running_idx, int time) {
    for (int i = 0; i < n; ++i) {
        if (processes[i].completed) {
            continue;
        }
        if (i == running_idx) {
            processes[i].current_waiting_streak = 0;
            continue;
        }
        if (processes[i].arrival_time <= time) {
            processes[i].current_waiting_streak++;
            if (processes[i].current_waiting_streak > processes[i].max_waiting_streak) {
                processes[i].max_waiting_streak = processes[i].current_waiting_streak;
            }
        }
    }
}

int run_fcfs(Process src[], int n, const char *scenario, Metrics *metrics) {
    Process processes[MAX_PROCESSES];
    TimelineEntry *timeline = NULL;
    int timeline_count = 0;
    int current_time = 0;
    int completed = 0;
    int running_idx = -1;
    int last_pid = -1;
    int context_switches = 0;
    int busy_time = 0;

    copy_processes(processes, src, n);
    timeline = (TimelineEntry *)calloc(MAX_TIMELINE, sizeof(TimelineEntry));
    if (timeline == NULL) {
        return 1;
    }

    while (completed < n) {
        if (running_idx < 0) {
            int best = -1;
            int best_arrival = 0;
            for (int i = 0; i < n; ++i) {
                if (processes[i].completed || processes[i].arrival_time > current_time) {
                    continue;
                }
                if (best < 0 || processes[i].arrival_time < best_arrival ||
                    (processes[i].arrival_time == best_arrival && processes[i].pid < processes[best].pid)) {
                    best = i;
                    best_arrival = processes[i].arrival_time;
                }
            }
            running_idx = best;
            if (running_idx >= 0 && last_pid != processes[running_idx].pid) {
                context_switches++;
                last_pid = processes[running_idx].pid;
            }
        }

        if (running_idx < 0) {
            record_timeline(timeline, &timeline_count, current_time, -1);
            update_waiting(processes, n, -1, current_time);
            current_time++;
            last_pid = -1;
            continue;
        }

        if (!processes[running_idx].started) {
            processes[running_idx].started = 1;
            processes[running_idx].start_time = current_time;
            processes[running_idx].response_time = current_time - processes[running_idx].arrival_time;
        }

        record_timeline(timeline, &timeline_count, current_time, processes[running_idx].pid);
        update_waiting(processes, n, running_idx, current_time);
        processes[running_idx].remaining_time--;
        processes[running_idx].total_run_time++;
        busy_time++;
        current_time++;

        if (processes[running_idx].remaining_time == 0) {
            processes[running_idx].finish_time = current_time;
            processes[running_idx].completed = 1;
            completed++;
            running_idx = -1;
        }
    }

    compute_metrics(metrics, "fcfs", scenario, processes, n, context_switches, current_time, busy_time);
    write_process_stats_csv("fcfs", scenario, processes, n);
    write_timeline_csv("fcfs", scenario, timeline, timeline_count);
    free(timeline);
    return 0;
}
