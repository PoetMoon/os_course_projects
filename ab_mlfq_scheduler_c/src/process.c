#include "process.h"

#include <stdio.h>
#include <string.h>

void init_process(Process *p) {
    memset(p, 0, sizeof(*p));
    p->start_time = -1;
    p->finish_time = -1;
    p->response_time = -1;
    p->completed = 0;
    p->started = 0;
}

void copy_processes(Process dest[], Process src[], int n) {
    for (int i = 0; i < n; ++i) {
        dest[i] = src[i];
    }
}

void init_queue(Queue *q) {
    q->front = 0;
    q->rear = 0;
    q->size = 0;
}

int is_empty(Queue *q) {
    return q->size == 0;
}

void enqueue(Queue *q, int pid_index) {
    if (q->size >= MAX_PROCESSES) {
        return;
    }
    q->items[q->rear] = pid_index;
    q->rear = (q->rear + 1) % MAX_PROCESSES;
    q->size++;
}

int dequeue(Queue *q) {
    int value = -1;
    if (q->size == 0) {
        return -1;
    }
    value = q->items[q->front];
    q->front = (q->front + 1) % MAX_PROCESSES;
    q->size--;
    return value;
}

int remove_from_queue(Queue *q, int pid_index) {
    int removed = 0;
    int temp[MAX_PROCESSES];
    int temp_size = 0;
    int original_size = q->size;

    for (int i = 0; i < original_size; ++i) {
        int value = dequeue(q);
        if (!removed && value == pid_index) {
            removed = 1;
            continue;
        }
        temp[temp_size++] = value;
    }

    init_queue(q);
    for (int i = 0; i < temp_size; ++i) {
        enqueue(q, temp[i]);
    }
    return removed;
}
