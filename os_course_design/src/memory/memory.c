#include "memory.h"

#include "common.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_PARTITIONS 32
#define MAX_FRAMES 16
#define MAX_REFERENCES 128

typedef struct {
    int start;
    int size;
    int free;
    int job_id;
} Partition;

static void print_partitions(const Partition *parts, int count) {
    printf("\n%-8s%-10s%-10s%-10s%-10s\n", "编号", "起始地址", "大小", "状态", "作业");
    for (int i = 0; i < count; ++i) {
        printf("%-8d%-10d%-10d%-10s%-10d\n",
               i,
               parts[i].start,
               parts[i].size,
               parts[i].free ? "空闲" : "占用",
               parts[i].job_id);
    }
}

static void merge_partitions(Partition *parts, int *count) {
    for (int i = 0; i < *count - 1; ++i) {
        if (parts[i].free && parts[i + 1].free) {
            parts[i].size += parts[i + 1].size;
            for (int j = i + 1; j < *count - 1; ++j) {
                parts[j] = parts[j + 1];
            }
            --(*count);
            --i;
        }
    }
}

static void allocate_partition(Partition *parts, int *count, int index, int job_id, int request) {
    if (parts[index].size == request) {
        parts[index].free = 0;
        parts[index].job_id = job_id;
        return;
    }

    for (int i = *count; i > index + 1; --i) {
        parts[i] = parts[i - 1];
    }
    parts[index + 1].start = parts[index].start + request;
    parts[index + 1].size = parts[index].size - request;
    parts[index + 1].free = 1;
    parts[index + 1].job_id = -1;

    parts[index].size = request;
    parts[index].free = 0;
    parts[index].job_id = job_id;
    ++(*count);
}

static void simulate_partition_allocation(int use_best_fit) {
    Partition parts[MAX_PARTITIONS] = {{0, 640, 1, -1}};
    int count = 1;
    int choice = -1;

    while (choice != 0) {
        print_divider(use_best_fit ? "动态分区管理 BF" : "动态分区管理 FF");
        print_partitions(parts, count);
        printf("\n1. 分配内存\n");
        printf("2. 回收内存\n");
        printf("0. 返回\n");
        choice = read_int("请选择操作: ");

        if (choice == 1) {
            int job_id = read_int("作业编号: ");
            int request = read_int("申请大小: ");
            int selected = -1;

            if (job_id <= 0 || request <= 0) {
                printf("作业编号和申请大小必须大于 0。\n");
                continue;
            }

            if (use_best_fit) {
                int min_size = INT_MAX;
                for (int i = 0; i < count; ++i) {
                    if (parts[i].free && parts[i].size >= request && parts[i].size < min_size) {
                        min_size = parts[i].size;
                        selected = i;
                    }
                }
            } else {
                for (int i = 0; i < count; ++i) {
                    if (parts[i].free && parts[i].size >= request) {
                        selected = i;
                        break;
                    }
                }
            }

            if (selected < 0 || count >= MAX_PARTITIONS) {
                printf("分配失败：没有合适的空闲分区。\n");
            } else {
                allocate_partition(parts, &count, selected, job_id, request);
                printf("分配成功。\n");
            }
        } else if (choice == 2) {
            int job_id = read_int("请输入要回收的作业编号: ");
            int found = 0;
            for (int i = 0; i < count; ++i) {
                if (!parts[i].free && parts[i].job_id == job_id) {
                    parts[i].free = 1;
                    parts[i].job_id = -1;
                    found = 1;
                    break;
                }
            }
            if (!found) {
                printf("未找到对应作业。\n");
            } else {
                merge_partitions(parts, &count);
                printf("回收成功。\n");
            }
        }
    }
}

static void print_frames(const int *frames, int frame_count) {
    printf("页框状态: ");
    for (int i = 0; i < frame_count; ++i) {
        if (frames[i] >= 0) {
            printf("[%d] ", frames[i]);
        } else {
            printf("[ ] ");
        }
    }
    printf("\n");
}

static void simulate_page_replacement(int use_lru) {
    int frame_count = read_int("请输入页框数量: ");
    int ref_count = read_int("请输入页面引用串长度: ");
    int refs[MAX_REFERENCES];
    int frames[MAX_FRAMES];
    int aux[MAX_FRAMES];
    int page_faults = 0;
    int clock = 0;

    if (frame_count <= 0 || frame_count > MAX_FRAMES || ref_count <= 0 || ref_count > MAX_REFERENCES) {
        printf("输入超出范围。\n");
        return;
    }

    for (int i = 0; i < ref_count; ++i) {
        char prompt[64];
        snprintf(prompt, sizeof(prompt), "第 %d 次访问的页号: ", i + 1);
        refs[i] = read_int(prompt);
    }

    for (int i = 0; i < frame_count; ++i) {
        frames[i] = -1;
        aux[i] = -1;
    }

    print_divider(use_lru ? "LRU 页面置换" : "FIFO 页面置换");
    for (int i = 0; i < ref_count; ++i) {
        int page = refs[i];
        int hit = 0;
        int empty = -1;
        int victim = 0;

        printf("访问页 %d: ", page);
        for (int j = 0; j < frame_count; ++j) {
            if (frames[j] == page) {
                hit = 1;
                if (use_lru) {
                    aux[j] = clock;
                }
                break;
            }
            if (frames[j] == -1 && empty < 0) {
                empty = j;
            }
        }

        if (hit) {
            printf("命中\n");
        } else {
            ++page_faults;
            if (empty >= 0) {
                victim = empty;
            } else {
                int best = aux[0];
                victim = 0;
                for (int j = 1; j < frame_count; ++j) {
                    if (aux[j] < best) {
                        best = aux[j];
                        victim = j;
                    }
                }
            }

            frames[victim] = page;
            aux[victim] = clock;
            printf("缺页，替换到页框 %d\n", victim);
        }

        print_frames(frames, frame_count);
        ++clock;
    }

    printf("\n缺页次数: %d\n", page_faults);
    printf("缺页率: %.2f%%\n", 100.0 * (double)page_faults / (double)ref_count);
}

void memory_menu(void) {
    int choice = -1;

    while (choice != 0) {
        print_divider("内存管理");
        printf("1. 首次适应 FF\n");
        printf("2. 最佳适应 BF\n");
        printf("3. FIFO 页面置换\n");
        printf("4. LRU 页面置换\n");
        printf("0. 返回上一级\n");
        choice = read_int("请选择实验: ");

        switch (choice) {
            case 1:
                simulate_partition_allocation(0);
                break;
            case 2:
                simulate_partition_allocation(1);
                break;
            case 3:
                simulate_page_replacement(0);
                break;
            case 4:
                simulate_page_replacement(1);
                break;
            case 0:
                break;
            default:
                printf("无效选项。\n");
                break;
        }
    }
}
