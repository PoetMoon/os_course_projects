#include "csv_writer.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

static void make_dir_if_missing(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        mkdir(path, 0777);
    }
}

void ensure_output_dirs(void) {
    make_dir_if_missing("results");
    make_dir_if_missing("figures");
}

void init_summary_csv(void) {
    FILE *fp = NULL;
    ensure_output_dirs();
    fp = fopen("results/summary.csv", "w");
    if (fp == NULL) {
        return;
    }
    fprintf(fp,
            "scenario,algorithm,average_turnaround_time,average_weighted_turnaround_time,"
            "average_waiting_time,average_response_time,max_waiting_time,waiting_time_variance,"
            "context_switch_count,cpu_utilization,throughput,starvation_count,starvation_rate,"
            "fairness_index\n");
    fclose(fp);
}

void append_summary_csv(const Metrics *metrics) {
    FILE *fp = fopen("results/summary.csv", "a");
    if (fp == NULL) {
        return;
    }
    fprintf(fp,
            "%s,%s,%.6f,%.6f,%.6f,%.6f,%d,%.6f,%d,%.6f,%.6f,%d,%.6f,%.6f\n",
            metrics->scenario,
            metrics->algorithm,
            metrics->average_turnaround_time,
            metrics->average_weighted_turnaround_time,
            metrics->average_waiting_time,
            metrics->average_response_time,
            metrics->max_waiting_time,
            metrics->waiting_time_variance,
            metrics->context_switch_count,
            metrics->cpu_utilization,
            metrics->throughput,
            metrics->starvation_count,
            metrics->starvation_rate,
            metrics->fairness_index);
    fclose(fp);
}

void write_process_stats_csv(
    const char *algorithm,
    const char *scenario,
    Process processes[],
    int n
) {
    char path[256];
    FILE *fp = NULL;

    snprintf(path, sizeof(path), "results/process_stats_%s_%s.csv", algorithm, scenario);
    fp = fopen(path, "w");
    if (fp == NULL) {
        return;
    }

    fprintf(fp,
            "pid,arrival_time,burst_time,finish_time,turnaround_time,waiting_time,response_time,"
            "queue_level,demotion_count,promotion_count,max_waiting_streak\n");
    for (int i = 0; i < n; ++i) {
        fprintf(fp,
                "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                processes[i].pid,
                processes[i].arrival_time,
                processes[i].burst_time,
                processes[i].finish_time,
                processes[i].turnaround_time,
                processes[i].waiting_time,
                processes[i].response_time,
                processes[i].queue_level,
                processes[i].demotion_count,
                processes[i].promotion_count,
                processes[i].max_waiting_streak);
    }
    fclose(fp);
}

void write_timeline_csv(
    const char *algorithm,
    const char *scenario,
    const TimelineEntry timeline[],
    int timeline_count
) {
    char path[256];
    FILE *fp = NULL;

    snprintf(path, sizeof(path), "results/timeline_%s_%s.csv", algorithm, scenario);
    fp = fopen(path, "w");
    if (fp == NULL) {
        return;
    }

    fprintf(fp, "time,pid,algorithm,queue\n");
    for (int i = 0; i < timeline_count; ++i) {
        fprintf(fp, "%d,%d,%s,%d\n",
                timeline[i].time,
                timeline[i].pid,
                timeline[i].algorithm,
                timeline[i].queue);
    }
    fclose(fp);
}

void write_ab_debug_csv(
    const char *scenario,
    const DebugEntry debug_entries[],
    int debug_count
) {
    char path[256];
    FILE *fp = NULL;

    snprintf(path, sizeof(path), "results/ab_mlfq_debug_%s.csv", scenario);
    fp = fopen(path, "w");
    if (fp == NULL) {
        return;
    }

    fprintf(fp, "time,event,pid,old_queue,new_queue,detail\n");
    for (int i = 0; i < debug_count; ++i) {
        fprintf(fp,
                "%d,%s,%d,%d,%d,%s\n",
                debug_entries[i].time,
                debug_entries[i].event,
                debug_entries[i].pid,
                debug_entries[i].old_queue,
                debug_entries[i].new_queue,
                debug_entries[i].detail);
    }
    fclose(fp);
}
