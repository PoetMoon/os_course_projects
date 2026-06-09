#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stddef.h>

#define MAX_NAME_LEN 64
#define MAX_INPUT_LEN 512

void clear_input_buffer(void);
int read_int(const char *prompt);
void read_string(const char *prompt, char *buffer, size_t size);
void pause_and_wait(void);
void print_divider(const char *title);

#endif
