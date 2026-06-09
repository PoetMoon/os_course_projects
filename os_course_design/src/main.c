#include "common.h"
#include "filesystem.h"
#include "memory.h"
#include "scheduler.h"
#include "sync.h"

#include <stdio.h>

static void show_main_menu(void) {
    print_divider("操作系统课程设计统一平台");
    printf("1. 处理机调度\n");
    printf("2. 内存管理\n");
    printf("3. 进程同步与并发控制\n");
    printf("4. 模拟文件系统\n");
    printf("0. 退出程序\n");
}

int main(void) {
    int choice = -1;

    while (choice != 0) {
        show_main_menu();
        choice = read_int("请选择模块: ");

        switch (choice) {
            case 1:
                scheduler_menu();
                break;
            case 2:
                memory_menu();
                break;
            case 3:
                sync_menu();
                break;
            case 4:
                filesystem_menu();
                break;
            case 0:
                printf("程序结束。\n");
                break;
            default:
                printf("无效选项，请重新选择。\n");
                break;
        }
    }

    return 0;
}
