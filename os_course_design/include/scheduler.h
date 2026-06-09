#ifndef SCHEDULER_H
#define SCHEDULER_H

typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int priority;
    int remaining_time;
    int completion_time;
    int turnaround_time;
    double weighted_turnaround_time;
    int finished;
} Process;

void scheduler_menu(void);

#endif
