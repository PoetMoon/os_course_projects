#include "scheduler.h"

#include <stdlib.h>
#include <string.h>

static void record_timeline(TimelineEntry timeline[], int *count, int time, int pid, int queue_level) {
    if (*count >= MAX_TIMELINE) {
        return;
    }
    timeline[*count].time = time;
    timeline[*count].pid = pid;
    strcpy(timeline[*count].algorithm, "rr");
    timeline[*count].queue = queue_level;
    (*count)++;
}

static void update_ready_queue_waiting(Queue *q, Process processes[]) {
    int size = q->size;
    for (int i = 0; i < size; ++i) {
        int idx = dequeue(q);
        processes[idx].current_waiting_streak++;
        if (processes[idx].current_waiting_streak > processes[idx].max_waiting_streak) {
            processes[idx].max_waiting_streak = processes[idx].current_waiting_streak;
        }
        enqueue(q, idx);
    }
}

int run_rr(Process src[], int n, const char *scenario, Metrics *metrics) {
    Process processes[MAX_PROCESSES];
    TimelineEntry *timeline = NULL;
    Queue ready_queue;
    int admitted[MAX_PROCESSES] = {0};
    int timeline_count = 0;
    int current_time = 0;
    int completed = 0;
    int running_idx = -1;
    int quantum = 4;
    int slice_used = 0;
    int last_pid = -1;
    int context_switches = 0;
    int busy_time = 0;

    copy_processes(processes, src, n);
    init_queue(&ready_queue);
    timeline = (TimelineEntry *)calloc(MAX_TIMELINE, sizeof(TimelineEntry));
    if (timeline == NULL) {
        return 1;
    }

    while (completed < n) {
        for (int i = 0; i < n; ++i) {
            if (!admitted[i] && processes[i].arrival_time <= current_time) {
                enqueue(&ready_queue, i);
                admitted[i] = 1;
                processes[i].last_ready_time = current_time;
            }
        }

        if (running_idx < 0 && !is_empty(&ready_queue)) {
            running_idx = dequeue(&ready_queue);
            slice_used = 0;
            processes[running_idx].current_waiting_streak = 0;
            if (last_pid != processes[running_idx].pid) {
                context_switches++;
                last_pid = processes[running_idx].pid;
            }
        }

        if (running_idx < 0) {
            record_timeline(timeline, &timeline_count, current_time, -1, -1);
            update_ready_queue_waiting(&ready_queue, processes);
            current_time++;
            last_pid = -1;
            continue;
        }

        if (!processes[running_idx].started) {
            processes[running_idx].started = 1;
            processes[running_idx].start_time = current_time;
            processes[running_idx].response_time = current_time - processes[running_idx].arrival_time;
        }

        record_timeline(timeline, &timeline_count, current_time, processes[running_idx].pid, 0);
        update_ready_queue_waiting(&ready_queue, processes);
        processes[running_idx].current_waiting_streak = 0;
        processes[running_idx].remaining_time--;
        processes[running_idx].total_run_time++;
        slice_used++;
        busy_time++;
        current_time++;

        if (processes[running_idx].remaining_time == 0) {
            processes[running_idx].finish_time = current_time;
            processes[running_idx].completed = 1;
            completed++;
            running_idx = -1;
        } else if (slice_used >= quantum) {
            enqueue(&ready_queue, running_idx);
            processes[running_idx].last_ready_time = current_time;
            running_idx = -1;
        }
    }

    compute_metrics(metrics, "rr", scenario, processes, n, context_switches, current_time, busy_time);
    write_process_stats_csv("rr", scenario, processes, n);
    write_timeline_csv("rr", scenario, timeline, timeline_count);
    free(timeline);
    return 0;
}
