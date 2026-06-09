#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "csv_writer.h"
#include "metrics.h"
#include "process.h"

int run_fcfs(Process src[], int n, const char *scenario, Metrics *metrics);
int run_sjf(Process src[], int n, const char *scenario, Metrics *metrics);
int run_rr(Process src[], int n, const char *scenario, Metrics *metrics);
int run_mlfq(Process src[], int n, const char *scenario, Metrics *metrics);
int run_ab_mlfq(Process src[], int n, const char *scenario, Metrics *metrics);

#endif
