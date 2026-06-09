#ifndef PROCESS_H
#define PROCESS_H

#define MAX_PROCESSES 512
#define MAX_NAME_LEN 32
#define MAX_TIMELINE 200000
#define MAX_DEBUG_EVENTS 200000

typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int priority;

    char process_type[32];

    int start_time;
    int finish_time;
    int response_time;
    int turnaround_time;
    int waiting_time;

    int total_run_time;
    int last_ready_time;

    int queue_level;
    int demotion_count;
    int promotion_count;

    int used_full_quantum_count;
    int early_yield_count;

    double aging_score;
    int max_waiting_streak;
    int current_waiting_streak;

    int completed;
    int started;
} Process;

typedef struct {
    int items[MAX_PROCESSES];
    int front;
    int rear;
    int size;
} Queue;

void init_process(Process *p);
void copy_processes(Process dest[], Process src[], int n);

void init_queue(Queue *q);
int is_empty(Queue *q);
void enqueue(Queue *q, int pid_index);
int dequeue(Queue *q);
int remove_from_queue(Queue *q, int pid_index);

#endif
