#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void clear_input_buffer(void) {
    int ch = 0;
    while ((ch = getchar()) != '\n' && ch != EOF) {
    }
}

int read_int(const char *prompt) {
    char line[MAX_INPUT_LEN];
    char *end = NULL;
    long value = 0;

    while (1) {
        if (prompt != NULL) {
            printf("%s", prompt);
        }

        if (fgets(line, sizeof(line), stdin) == NULL) {
            return 0;
        }

        value = strtol(line, &end, 10);
        if (end != line) {
            while (*end == ' ' || *end == '\t') {
                ++end;
            }
            if (*end == '\n' || *end == '\0') {
                return (int)value;
            }
        }
        printf("输入无效，请重新输入整数。\n");
    }
}

void read_string(const char *prompt, char *buffer, size_t size) {
    if (prompt != NULL) {
        printf("%s", prompt);
    }

    if (fgets(buffer, size, stdin) == NULL) {
        if (size > 0) {
            buffer[0] = '\0';
        }
        return;
    }

    buffer[strcspn(buffer, "\n")] = '\0';
}

void pause_and_wait(void) {
    printf("\n按回车继续...");
    fflush(stdout);
    clear_input_buffer();
}

void print_divider(const char *title) {
    printf("\n================ %s ================\n", title);
}
