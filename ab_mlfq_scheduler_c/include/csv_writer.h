#ifndef CSV_WRITER_H
#define CSV_WRITER_H

#include "metrics.h"
#include "process.h"

typedef struct {
    int time;
    int pid;
    char algorithm[32];
    int queue;
} TimelineEntry;

typedef struct {
    int time;
    char event[32];
    int pid;
    int old_queue;
    int new_queue;
    char detail[128];
} DebugEntry;

void ensure_output_dirs(void);
void init_summary_csv(void);
void append_summary_csv(const Metrics *metrics);
void write_process_stats_csv(
    const char *algorithm,
    const char *scenario,
    Process processes[],
    int n
);
void write_timeline_csv(
    const char *algorithm,
    const char *scenario,
    const TimelineEntry timeline[],
    int timeline_count
);
void write_ab_debug_csv(
    const char *scenario,
    const DebugEntry debug_entries[],
    int debug_count
);

#endif
