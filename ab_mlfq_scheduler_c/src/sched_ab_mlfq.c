#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void record_timeline(TimelineEntry timeline[], int *count, int time, int pid, int queue_level) {
    if (*count >= MAX_TIMELINE) {
        return;
    }
    timeline[*count].time = time;
    timeline[*count].pid = pid;
    strcpy(timeline[*count].algorithm, "ab_mlfq");
    timeline[*count].queue = queue_level;
    (*count)++;
}

static void record_debug(
    DebugEntry debug_entries[],
    int *count,
    int time,
    const char *event,
    int pid,
    int old_queue,
    int new_queue,
    const char *detail
) {
    if (*count >= MAX_DEBUG_EVENTS) {
        return;
    }
    debug_entries[*count].time = time;
    strncpy(debug_entries[*count].event, event, sizeof(debug_entries[*count].event) - 1);
    debug_entries[*count].pid = pid;
    debug_entries[*count].old_queue = old_queue;
    debug_entries[*count].new_queue = new_queue;
    strncpy(debug_entries[*count].detail, detail, sizeof(debug_entries[*count].detail) - 1);
    (*count)++;
}

static int highest_nonempty_queue(Queue queues[]) {
    for (int i = 0; i < 4; ++i) {
        if (!is_empty(&queues[i])) {
            return i;
        }
    }
    return -1;
}

static int has_type(const Process *process, const char *token) {
    return strstr(process->process_type, token) != NULL;
}

static int initial_queue_for(const Process *process) {
    if (has_type(process, "latency") || has_type(process, "interactive")) {
        return 0;
    }
    if (has_type(process, "cpu_bound") || has_type(process, "batch")) {
        return 2;
    }
    return 0;
}

static int queue_has_latency_process(Queue *queue, Process processes[]) {
    for (int i = 0; i < queue->size; ++i) {
        int pos = (queue->front + i) % MAX_PROCESSES;
        int idx = queue->items[pos];
        if (has_type(&processes[idx], "latency") || has_type(&processes[idx], "interactive")) {
            return 1;
        }
    }
    return 0;
}

static int highest_latency_queue_above(Queue queues[], Process processes[], int current_level) {
    for (int level = 0; level < current_level; ++level) {
        if (queue_has_latency_process(&queues[level], processes)) {
            return level;
        }
    }
    return -1;
}

static double safe_ratio(int a, int b) {
    int total = a + b;
    if (total <= 0) {
        return 0.0;
    }
    return (double)a / (double)total;
}

static double current_starvation_rate(Process processes[], int n) {
    int count = 0;
    for (int i = 0; i < n; ++i) {
        if (processes[i].max_waiting_streak >= 30) {
            count++;
        }
    }
    return n > 0 ? (double)count / (double)n : 0.0;
}

static void apply_behavior_aging(
    Queue queues[],
    Process processes[],
    double aging_trigger_score,
    DebugEntry debug_entries[],
    int *debug_count,
    int current_time
) {
    for (int level = 3; level >= 0; --level) {
        int size = queues[level].size;
        for (int i = 0; i < size; ++i) {
            int idx = dequeue(&queues[level]);
            int old_level = processes[idx].queue_level;
            double increment = 0.2;
            char detail[128];

            processes[idx].current_waiting_streak++;
            if (processes[idx].current_waiting_streak > processes[idx].max_waiting_streak) {
                processes[idx].max_waiting_streak = processes[idx].current_waiting_streak;
            }

            if (processes[idx].current_waiting_streak < 10) {
                increment = 0.2;
            } else if (processes[idx].current_waiting_streak < 30) {
                increment = 0.6;
            } else {
                increment = 1.2;
            }
            processes[idx].aging_score += increment;

            if (processes[idx].aging_score >= aging_trigger_score && old_level > 0) {
                processes[idx].queue_level = old_level - 1;
                processes[idx].promotion_count++;
                processes[idx].aging_score = 0.0;
                processes[idx].current_waiting_streak = 0;
                snprintf(detail, sizeof(detail), "aging_score trigger %.2f", aging_trigger_score);
                record_debug(
                    debug_entries, debug_count, current_time, "aging_promotion",
                    processes[idx].pid, old_level, processes[idx].queue_level, detail
                );
                enqueue(&queues[processes[idx].queue_level], idx);
            } else {
                enqueue(&queues[level], idx);
            }
        }
    }
}

int run_ab_mlfq(Process src[], int n, const char *scenario, Metrics *metrics) {
    Process processes[MAX_PROCESSES];
    TimelineEntry *timeline = NULL;
    DebugEntry *debug_entries = NULL;
    Queue queues[4];
    int admitted[MAX_PROCESSES] = {0};
    int quantum[4] = {2, 4, 8, 16};
    int adjust_interval = 20;
    int timeline_count = 0;
    int debug_count = 0;
    int current_time = 0;
    int completed = 0;
    int running_idx = -1;
    int slice_used = 0;
    int last_pid = -1;
    int context_switches = 0;
    int busy_time = 0;
    int window_context_switches = 0;
    int window_response_sum = 0;
    int window_response_count = 0;
    double aging_trigger_score = 8.0;

    copy_processes(processes, src, n);
    timeline = (TimelineEntry *)calloc(MAX_TIMELINE, sizeof(TimelineEntry));
    debug_entries = (DebugEntry *)calloc(MAX_DEBUG_EVENTS, sizeof(DebugEntry));
    if (timeline == NULL || debug_entries == NULL) {
        free(timeline);
        free(debug_entries);
        return 1;
    }
    for (int i = 0; i < 4; ++i) {
        init_queue(&queues[i]);
    }

    while (completed < n) {
        char detail[128];

        for (int i = 0; i < n; ++i) {
            if (!admitted[i] && processes[i].arrival_time <= current_time) {
                int initial_level = initial_queue_for(&processes[i]);
                processes[i].queue_level = initial_level;
                enqueue(&queues[initial_level], i);
                admitted[i] = 1;
                processes[i].last_ready_time = current_time;
                snprintf(detail, sizeof(detail), "type=%s enters Q%d", processes[i].process_type, initial_level);
                record_debug(debug_entries, &debug_count, current_time, "arrival",
                             processes[i].pid, -1, initial_level, detail);
            }
        }

        apply_behavior_aging(
            queues,
            processes,
            aging_trigger_score,
            debug_entries,
            &debug_count,
            current_time
        );

        if (running_idx >= 0) {
            int highest_ready = highest_latency_queue_above(
                queues,
                processes,
                processes[running_idx].queue_level
            );
            if (highest_ready >= 0) {
                int old_level = processes[running_idx].queue_level;
                enqueue(&queues[old_level], running_idx);
                processes[running_idx].last_ready_time = current_time;
                snprintf(detail, sizeof(detail), "preempted by Q%d ready process", highest_ready);
                record_debug(debug_entries, &debug_count, current_time, "priority_preempt",
                             processes[running_idx].pid, old_level, old_level, detail);
                running_idx = -1;
                slice_used = 0;
            }
        }

        if (running_idx < 0) {
            int level = highest_nonempty_queue(queues);
            if (level >= 0) {
                running_idx = dequeue(&queues[level]);
                processes[running_idx].queue_level = level;
                processes[running_idx].current_waiting_streak = 0;
                slice_used = 0;
                if (last_pid != processes[running_idx].pid) {
                    context_switches++;
                    window_context_switches++;
                    last_pid = processes[running_idx].pid;
                }
            }
        }

        if (running_idx < 0) {
            record_timeline(timeline, &timeline_count, current_time, -1, -1);
            current_time++;
        } else {
            int old_level = processes[running_idx].queue_level;
            int consumed_full_quantum = 0;
            double cpu_intensity_score = 0.0;
            double interactive_score = 0.0;

            if (!processes[running_idx].started) {
                processes[running_idx].started = 1;
                processes[running_idx].start_time = current_time;
                processes[running_idx].response_time = current_time - processes[running_idx].arrival_time;
                window_response_sum += processes[running_idx].response_time;
                window_response_count++;
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
                if (slice_used < quantum[old_level]) {
                    processes[running_idx].early_yield_count++;
                    interactive_score = safe_ratio(
                        processes[running_idx].early_yield_count,
                        processes[running_idx].used_full_quantum_count
                    );
                    if (interactive_score >= 0.6 && old_level > 0) {
                        processes[running_idx].promotion_count++;
                        snprintf(detail, sizeof(detail), "interactive_score=%.2f, completion before quantum", interactive_score);
                        record_debug(debug_entries, &debug_count, current_time, "interactive_hint",
                                     processes[running_idx].pid, old_level, old_level - 1, detail);
                    }
                }
                processes[running_idx].finish_time = current_time;
                processes[running_idx].completed = 1;
                completed++;
                running_idx = -1;
            } else if (slice_used >= quantum[old_level]) {
                consumed_full_quantum = 1;
                processes[running_idx].used_full_quantum_count++;
                cpu_intensity_score = safe_ratio(
                    processes[running_idx].used_full_quantum_count,
                    processes[running_idx].early_yield_count
                );

                if (cpu_intensity_score >= 0.7 && processes[running_idx].queue_level < 3) {
                    int step = has_type(&processes[running_idx], "cpu_bound") ||
                               has_type(&processes[running_idx], "batch")
                                   ? 2
                                   : 1;
                    processes[running_idx].queue_level += step;
                    if (processes[running_idx].queue_level > 3) {
                        processes[running_idx].queue_level = 3;
                    }
                    processes[running_idx].demotion_count++;
                    snprintf(detail, sizeof(detail), "cpu_intensity_score=%.2f", cpu_intensity_score);
                    record_debug(debug_entries, &debug_count, current_time, "behavior_demotion",
                                 processes[running_idx].pid, old_level, processes[running_idx].queue_level, detail);
                } else {
                    snprintf(detail, sizeof(detail), "cpu_intensity_score=%.2f, queue unchanged", cpu_intensity_score);
                    record_debug(debug_entries, &debug_count, current_time, "behavior_keep",
                                 processes[running_idx].pid, old_level, processes[running_idx].queue_level, detail);
                }

                enqueue(&queues[processes[running_idx].queue_level], running_idx);
                processes[running_idx].last_ready_time = current_time;
                slice_used = 0;
                running_idx = -1;
            } else {
                (void)consumed_full_quantum;
            }
        }

        if (current_time > 0 && current_time % adjust_interval == 0) {
            double avg_response = window_response_count > 0
                                      ? (double)window_response_sum / (double)window_response_count
                                      : 0.0;
            double starvation_rate_now = current_starvation_rate(processes, n);

            if (window_context_switches > 10) {
                if (quantum[2] < 12) {
                    quantum[2]++;
                }
                if (quantum[3] < 24) {
                    quantum[3]++;
                }
            }
            if (avg_response > 10.0) {
                if (quantum[0] > 1) {
                    quantum[0]--;
                }
                if (quantum[1] > 2) {
                    quantum[1]--;
                }
            }

            aging_trigger_score = starvation_rate_now > 0.2 ? 6.0 : 8.0;
            snprintf(detail, sizeof(detail),
                     "Q0=%d,Q1=%d,Q2=%d,Q3=%d,aging_trigger=%.1f,window_cs=%d,avg_resp=%.2f",
                     quantum[0], quantum[1], quantum[2], quantum[3], aging_trigger_score,
                     window_context_switches, avg_response);
            record_debug(debug_entries, &debug_count, current_time, "quantum_adjust", -1, -1, -1, detail);

            window_context_switches = 0;
            window_response_sum = 0;
            window_response_count = 0;
        }
    }

    compute_metrics(metrics, "ab_mlfq", scenario, processes, n, context_switches, current_time, busy_time);
    write_process_stats_csv("ab_mlfq", scenario, processes, n);
    write_timeline_csv("ab_mlfq", scenario, timeline, timeline_count);
    write_ab_debug_csv(scenario, debug_entries, debug_count);
    free(timeline);
    free(debug_entries);
    return 0;
}
