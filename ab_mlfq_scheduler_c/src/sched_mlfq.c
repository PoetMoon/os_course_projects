#include "scheduler.h"

#include <stdlib.h>
#include <string.h>

static void record_timeline(TimelineEntry timeline[], int *count, int time, int pid, int queue_level) {
    if (*count >= MAX_TIMELINE) {
        return;
    }
    timeline[*count].time = time;
    timeline[*count].pid = pid;
    strcpy(timeline[*count].algorithm, "mlfq");
    timeline[*count].queue = queue_level;
    (*count)++;
}

static int highest_nonempty_queue(Queue queues[], int levels) {
    for (int i = 0; i < levels; ++i) {
        if (!is_empty(&queues[i])) {
            return i;
        }
    }
    return -1;
}

static void apply_aging(Queue queues[], Process processes[], int aging_threshold) {
    for (int level = 1; level < 4; ++level) {
        int size = queues[level].size;
        for (int i = 0; i < size; ++i) {
            int idx = dequeue(&queues[level]);
            processes[idx].current_waiting_streak++;
            if (processes[idx].current_waiting_streak > processes[idx].max_waiting_streak) {
                processes[idx].max_waiting_streak = processes[idx].current_waiting_streak;
            }
            if (processes[idx].current_waiting_streak > aging_threshold && level > 0) {
                processes[idx].queue_level = level - 1;
                processes[idx].promotion_count++;
                processes[idx].current_waiting_streak = 0;
                enqueue(&queues[level - 1], idx);
            } else {
                enqueue(&queues[level], idx);
            }
        }
    }

    {
        int size = queues[0].size;
        for (int i = 0; i < size; ++i) {
            int idx = dequeue(&queues[0]);
            processes[idx].current_waiting_streak++;
            if (processes[idx].current_waiting_streak > processes[idx].max_waiting_streak) {
                processes[idx].max_waiting_streak = processes[idx].current_waiting_streak;
            }
            enqueue(&queues[0], idx);
        }
    }
}

int run_mlfq(Process src[], int n, const char *scenario, Metrics *metrics) {
    Process processes[MAX_PROCESSES];
    TimelineEntry *timeline = NULL;
    Queue queues[4];
    int admitted[MAX_PROCESSES] = {0};
    int quantum[4] = {2, 4, 8, 16};
    int aging_threshold = 20;
    int timeline_count = 0;
    int current_time = 0;
    int completed = 0;
    int running_idx = -1;
    int slice_used = 0;
    int last_pid = -1;
    int context_switches = 0;
    int busy_time = 0;

    copy_processes(processes, src, n);
    timeline = (TimelineEntry *)calloc(MAX_TIMELINE, sizeof(TimelineEntry));
    if (timeline == NULL) {
        return 1;
    }
    for (int i = 0; i < 4; ++i) {
        init_queue(&queues[i]);
    }

    while (completed < n) {
        for (int i = 0; i < n; ++i) {
            if (!admitted[i] && processes[i].arrival_time <= current_time) {
                processes[i].queue_level = 0;
                enqueue(&queues[0], i);
                admitted[i] = 1;
                processes[i].last_ready_time = current_time;
            }
        }

        apply_aging(queues, processes, aging_threshold);

        if (running_idx < 0) {
            int level = highest_nonempty_queue(queues, 4);
            if (level >= 0) {
                running_idx = dequeue(&queues[level]);
                processes[running_idx].queue_level = level;
                processes[running_idx].current_waiting_streak = 0;
                slice_used = 0;
                if (last_pid != processes[running_idx].pid) {
                    context_switches++;
                    last_pid = processes[running_idx].pid;
                }
            }
        }

        if (running_idx < 0) {
            record_timeline(timeline, &timeline_count, current_time, -1, -1);
            current_time++;
            last_pid = -1;
            continue;
        }

        if (!processes[running_idx].started) {
            processes[running_idx].started = 1;
            processes[running_idx].start_time = current_time;
            processes[running_idx].response_time = current_time - processes[running_idx].arrival_time;
        }

        record_timeline(
            timeline,
            &timeline_count,
            current_time,
            processes[running_idx].pid,
            processes[running_idx].queue_level
        );

        processes[running_idx].remaining_time--;
        processes[running_idx].total_run_time++;
        processes[running_idx].current_waiting_streak = 0;
        slice_used++;
        busy_time++;
        current_time++;

        if (processes[running_idx].remaining_time == 0) {
            processes[running_idx].finish_time = current_time;
            processes[running_idx].completed = 1;
            completed++;
            running_idx = -1;
            continue;
        }

        if (slice_used >= quantum[processes[running_idx].queue_level]) {
            if (processes[running_idx].queue_level < 3) {
                processes[running_idx].queue_level++;
                processes[running_idx].demotion_count++;
            }
            enqueue(&queues[processes[running_idx].queue_level], running_idx);
            processes[running_idx].last_ready_time = current_time;
            running_idx = -1;
        }
    }

    compute_metrics(metrics, "mlfq", scenario, processes, n, context_switches, current_time, busy_time);
    write_process_stats_csv("mlfq", scenario, processes, n);
    write_timeline_csv("mlfq", scenario, timeline, timeline_count);
    free(timeline);
    return 0;
}
