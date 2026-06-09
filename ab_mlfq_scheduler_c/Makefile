CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -Iinclude
LDFLAGS =

TARGET = scheduler

SRC = \
	src/main.c \
	src/process.c \
	src/workload.c \
	src/metrics.c \
	src/csv_writer.c \
	src/sched_fcfs.c \
	src/sched_sjf.c \
	src/sched_rr.c \
	src/sched_mlfq.c \
	src/sched_ab_mlfq.c

OBJ = $(SRC:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
