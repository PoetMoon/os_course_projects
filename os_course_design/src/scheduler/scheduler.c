#include "scheduler.h"

#include "common.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCHEDULER_SAMPLE_1 "data/scheduler_cases/case1.txt"
#define SCHEDULER_SAMPLE_2 "data/scheduler_cases/case2.txt"

typedef struct {
    int pid;
    int start_time;
    int end_time;
} GanttEntry;

static Process *input_processes(int *count) {
    int n = read_int("请输入进程数量: ");
    Process *processes = NULL;

    if (n <= 0) {
        printf("进程数量必须大于 0。\n");
        *count = 0;
        return NULL;
    }

    processes = (Process *)calloc((size_t)n, sizeof(Process));
    if (processes == NULL) {
        printf("内存申请失败。\n");
        *count = 0;
        return NULL;
    }

    for (int i = 0; i < n; ++i) {
        printf("\n录入进程 P%d\n", i + 1);
        processes[i].pid = i + 1;
        processes[i].arrival_time = read_int("到达时间: ");
        processes[i].burst_time = read_int("服务时间: ");
        processes[i].priority = read_int("优先级(数字越小优先级越高): ");
        if (processes[i].arrival_time < 0 || processes[i].burst_time <= 0) {
            printf("到达时间不能小于 0，服务时间必须大于 0。\n");
            free(processes);
            *count = 0;
            return NULL;
        }
        processes[i].remaining_time = processes[i].burst_time;
    }

    *count = n;
    return processes;
}

static void print_process_source(const Process *processes, int count) {
    print_divider("当前调度数据");
    printf("%-8s%-10s%-10s%-10s\n", "进程", "到达", "服务", "优先级");
    for (int i = 0; i < count; ++i) {
        printf("P%-7d%-10d%-10d%-10d\n",
               processes[i].pid,
               processes[i].arrival_time,
               processes[i].burst_time,
               processes[i].priority);
    }
}

static Process *load_processes_from_file(const char *path, int *count) {
    FILE *fp = fopen(path, "r");
    Process *processes = NULL;
    int n = 0;

    if (fp == NULL) {
        printf("无法打开数据文件: %s\n", path);
        *count = 0;
        return NULL;
    }

    if (fscanf(fp, "%d", &n) != 1 || n <= 0) {
        printf("数据文件格式错误，首行应为正整数进程数。\n");
        fclose(fp);
        *count = 0;
        return NULL;
    }

    processes = (Process *)calloc((size_t)n, sizeof(Process));
    if (processes == NULL) {
        printf("内存申请失败。\n");
        fclose(fp);
        *count = 0;
        return NULL;
    }

    for (int i = 0; i < n; ++i) {
        int arrival = 0;
        int burst = 0;
        int priority = 0;
        if (fscanf(fp, "%d %d %d", &arrival, &burst, &priority) != 3) {
            printf("数据文件格式错误，第 %d 个进程缺少字段。\n", i + 1);
            free(processes);
            fclose(fp);
            *count = 0;
            return NULL;
        }
        processes[i].pid = i + 1;
        processes[i].arrival_time = arrival;
        processes[i].burst_time = burst;
        processes[i].priority = priority;
        if (arrival < 0 || burst <= 0) {
            printf("数据文件格式错误：到达时间不能小于 0，服务时间必须大于 0。\n");
            free(processes);
            *count = 0;
            return NULL;
        }
        processes[i].remaining_time = burst;
    }

    fclose(fp);
    *count = n;
    printf("已从文件加载调度数据: %s\n", path);
    return processes;
}

static Process *choose_process_source(int *count) {
    int choice = -1;
    Process *processes = NULL;
    char path[256];

    while (processes == NULL) {
        print_divider("调度数据来源");
        printf("1. 手动输入\n");
        printf("2. 读取标准样例 1\n");
        printf("3. 读取标准样例 2\n");
        printf("4. 读取自定义文件\n");
        printf("0. 返回上一级\n");
        choice = read_int("请选择数据来源: ");

        switch (choice) {
            case 1:
                processes = input_processes(count);
                break;
            case 2:
                processes = load_processes_from_file(SCHEDULER_SAMPLE_1, count);
                break;
            case 3:
                processes = load_processes_from_file(SCHEDULER_SAMPLE_2, count);
                break;
            case 4:
                read_string("请输入数据文件路径: ", path, sizeof(path));
                processes = load_processes_from_file(path, count);
                break;
            case 0:
                *count = 0;
                return NULL;
            default:
                printf("无效选项。\n");
                break;
        }
    }

    return processes;
}

static Process *clone_processes(const Process *src, int count) {
    Process *copy = (Process *)calloc((size_t)count, sizeof(Process));
    if (copy == NULL) {
        return NULL;
    }
    memcpy(copy, src, (size_t)count * sizeof(Process));
    for (int i = 0; i < count; ++i) {
        copy[i].remaining_time = copy[i].burst_time;
        copy[i].completion_time = 0;
        copy[i].turnaround_time = 0;
        copy[i].weighted_turnaround_time = 0.0;
        copy[i].finished = 0;
    }
    return copy;
}

static void finalize_metrics(Process *processes, int count) {
    for (int i = 0; i < count; ++i) {
        processes[i].turnaround_time =
            processes[i].completion_time - processes[i].arrival_time;
        processes[i].weighted_turnaround_time =
            (double)processes[i].turnaround_time / (double)processes[i].burst_time;
    }
}

static void print_results(const char *name, Process *processes, int count,
                          const GanttEntry *gantt, int gantt_count) {
    double avg_turnaround = 0.0;
    double avg_weighted = 0.0;

    print_divider(name);
    printf("运行顺序 / 甘特图:\n");
    for (int i = 0; i < gantt_count; ++i) {
        printf("[P%d: %d -> %d] ", gantt[i].pid, gantt[i].start_time, gantt[i].end_time);
    }
    printf("\n\n");

    printf("%-8s%-10s%-10s%-12s%-12s%-12s\n",
           "进程", "到达", "服务", "完成", "周转", "带权周转");
    for (int i = 0; i < count; ++i) {
        printf("P%-7d%-10d%-10d%-12d%-12d%-12.2f\n",
               processes[i].pid,
               processes[i].arrival_time,
               processes[i].burst_time,
               processes[i].completion_time,
               processes[i].turnaround_time,
               processes[i].weighted_turnaround_time);
        avg_turnaround += processes[i].turnaround_time;
        avg_weighted += processes[i].weighted_turnaround_time;
    }

    printf("\n平均周转时间: %.2f\n", avg_turnaround / count);
    printf("平均带权周转时间: %.2f\n", avg_weighted / count);
}

static void run_fcfs(Process *processes, int count) {
    GanttEntry *gantt = (GanttEntry *)calloc((size_t)count, sizeof(GanttEntry));
    int current_time = 0;
    int gantt_count = 0;

    for (int done = 0; done < count; ++done) {
        int idx = -1;
        int min_arrival = INT_MAX;
        for (int i = 0; i < count; ++i) {
            if (!processes[i].finished &&
                (processes[i].arrival_time < min_arrival ||
                 (processes[i].arrival_time == min_arrival &&
                  processes[i].pid < (idx >= 0 ? processes[idx].pid : INT_MAX)))) {
                min_arrival = processes[i].arrival_time;
                idx = i;
            }
        }

        if (idx < 0) {
            break;
        }

        if (current_time < processes[idx].arrival_time) {
            current_time = processes[idx].arrival_time;
        }
        gantt[gantt_count].pid = processes[idx].pid;
        gantt[gantt_count].start_time = current_time;
        current_time += processes[idx].burst_time;
        gantt[gantt_count].end_time = current_time;
        ++gantt_count;

        processes[idx].completion_time = current_time;
        processes[idx].finished = 1;
    }

    finalize_metrics(processes, count);
    print_results("先来先服务 FCFS", processes, count, gantt, gantt_count);
    free(gantt);
}

static void run_sjf(Process *processes, int count) {
    GanttEntry *gantt = (GanttEntry *)calloc((size_t)count, sizeof(GanttEntry));
    int current_time = 0;
    int completed = 0;
    int gantt_count = 0;

    while (completed < count) {
        int idx = -1;
        int best_burst = INT_MAX;
        int earliest = INT_MAX;
        for (int i = 0; i < count; ++i) {
            if (!processes[i].finished && processes[i].arrival_time <= current_time) {
                if (processes[i].burst_time < best_burst ||
                    (processes[i].burst_time == best_burst &&
                     processes[i].arrival_time < earliest)) {
                    best_burst = processes[i].burst_time;
                    earliest = processes[i].arrival_time;
                    idx = i;
                }
            }
        }

        if (idx < 0) {
            int next_arrival = INT_MAX;
            for (int i = 0; i < count; ++i) {
                if (!processes[i].finished && processes[i].arrival_time < next_arrival) {
                    next_arrival = processes[i].arrival_time;
                }
            }
            current_time = next_arrival;
            continue;
        }

        gantt[gantt_count].pid = processes[idx].pid;
        gantt[gantt_count].start_time = current_time;
        current_time += processes[idx].burst_time;
        gantt[gantt_count].end_time = current_time;
        ++gantt_count;

        processes[idx].completion_time = current_time;
        processes[idx].finished = 1;
        ++completed;
    }

    finalize_metrics(processes, count);
    print_results("短作业优先 SJF", processes, count, gantt, gantt_count);
    free(gantt);
}

static void run_priority(Process *processes, int count) {
    GanttEntry *gantt = (GanttEntry *)calloc((size_t)count, sizeof(GanttEntry));
    int current_time = 0;
    int completed = 0;
    int gantt_count = 0;

    while (completed < count) {
        int idx = -1;
        int best_priority = INT_MAX;
        int earliest = INT_MAX;
        for (int i = 0; i < count; ++i) {
            if (!processes[i].finished && processes[i].arrival_time <= current_time) {
                if (processes[i].priority < best_priority ||
                    (processes[i].priority == best_priority &&
                     processes[i].arrival_time < earliest)) {
                    best_priority = processes[i].priority;
                    earliest = processes[i].arrival_time;
                    idx = i;
                }
            }
        }

        if (idx < 0) {
            int next_arrival = INT_MAX;
            for (int i = 0; i < count; ++i) {
                if (!processes[i].finished && processes[i].arrival_time < next_arrival) {
                    next_arrival = processes[i].arrival_time;
                }
            }
            current_time = next_arrival;
            continue;
        }

        gantt[gantt_count].pid = processes[idx].pid;
        gantt[gantt_count].start_time = current_time;
        current_time += processes[idx].burst_time;
        gantt[gantt_count].end_time = current_time;
        ++gantt_count;

        processes[idx].completion_time = current_time;
        processes[idx].finished = 1;
        ++completed;
    }

    finalize_metrics(processes, count);
    print_results("优先级调度", processes, count, gantt, gantt_count);
    free(gantt);
}

static void run_rr_with_quantum(Process *processes, int count, int quantum) {
    GanttEntry *gantt = (GanttEntry *)calloc(512U, sizeof(GanttEntry));
    int gantt_capacity = 512;
    int gantt_count = 0;
    int current_time = 0;
    int completed = 0;
    int *queue = (int *)calloc((size_t)(count * 64), sizeof(int));
    int q_front = 0;
    int q_back = 0;
    int *in_queue = (int *)calloc((size_t)count, sizeof(int));

    if (quantum <= 0) {
        printf("时间片必须大于 0。\n");
        free(gantt);
        free(queue);
        free(in_queue);
        return;
    }

    while (completed < count) {
        for (int i = 0; i < count; ++i) {
            if (!processes[i].finished && !in_queue[i] &&
                processes[i].arrival_time <= current_time) {
                queue[q_back++] = i;
                in_queue[i] = 1;
            }
        }

        if (q_front == q_back) {
            current_time++;
            continue;
        }

        int idx = queue[q_front++];
        in_queue[idx] = 0;
        int slice = processes[idx].remaining_time < quantum
                        ? processes[idx].remaining_time
                        : quantum;

        if (gantt_count >= gantt_capacity) {
            gantt_capacity *= 2;
            gantt = (GanttEntry *)realloc(gantt, (size_t)gantt_capacity * sizeof(GanttEntry));
        }

        gantt[gantt_count].pid = processes[idx].pid;
        gantt[gantt_count].start_time = current_time;
        current_time += slice;
        gantt[gantt_count].end_time = current_time;
        ++gantt_count;

        processes[idx].remaining_time -= slice;

        for (int i = 0; i < count; ++i) {
            if (!processes[i].finished && !in_queue[i] &&
                processes[i].arrival_time <= current_time && i != idx) {
                queue[q_back++] = i;
                in_queue[i] = 1;
            }
        }

        if (processes[idx].remaining_time > 0) {
            queue[q_back++] = idx;
            in_queue[idx] = 1;
        } else {
            processes[idx].finished = 1;
            processes[idx].completion_time = current_time;
            ++completed;
        }
    }

    finalize_metrics(processes, count);
    print_results("时间片轮转 RR", processes, count, gantt, gantt_count);
    free(gantt);
    free(queue);
    free(in_queue);
}

static void run_rr(Process *processes, int count) {
    int quantum = read_int("请输入时间片大小: ");
    run_rr_with_quantum(processes, count, quantum);
}

void scheduler_menu(void) {
    int process_count = 0;
    Process *source = NULL;
    Process *work = NULL;
    int choice = -1;

    source = choose_process_source(&process_count);
    if (source == NULL || process_count == 0) {
        free(source);
        return;
    }

    while (choice != 0) {
        print_divider("处理机调度");
        print_process_source(source, process_count);
        printf("\n");
        printf("1. FCFS\n");
        printf("2. SJF\n");
        printf("3. RR\n");
        printf("4. 优先级调度\n");
        printf("5. 依次运行全部算法\n");
        printf("6. 更换调度数据\n");
        printf("0. 返回上一级\n");
        choice = read_int("请选择算法: ");

        switch (choice) {
            case 1:
                work = clone_processes(source, process_count);
                run_fcfs(work, process_count);
                free(work);
                break;
            case 2:
                work = clone_processes(source, process_count);
                run_sjf(work, process_count);
                free(work);
                break;
            case 3:
                work = clone_processes(source, process_count);
                run_rr(work, process_count);
                free(work);
                break;
            case 4:
                work = clone_processes(source, process_count);
                run_priority(work, process_count);
                free(work);
                break;
            case 5:
                {
                int quantum = read_int("请输入 RR 时间片大小: ");
                work = clone_processes(source, process_count);
                run_fcfs(work, process_count);
                free(work);
                work = clone_processes(source, process_count);
                run_sjf(work, process_count);
                free(work);
                work = clone_processes(source, process_count);
                run_rr_with_quantum(work, process_count, quantum);
                free(work);
                work = clone_processes(source, process_count);
                run_priority(work, process_count);
                free(work);
                }
                break;
            case 6:
                free(source);
                source = choose_process_source(&process_count);
                if (source == NULL || process_count == 0) {
                    return;
                }
                break;
            case 0:
                break;
            default:
                printf("无效选项。\n");
                break;
        }
    }

    free(source);
}
