CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Wno-deprecated-declarations -std=c11 -Iinclude
LDFLAGS = -pthread

TARGET = os_course_design

SRC = \
	src/main.c \
	src/common/utils.c \
	src/scheduler/scheduler.c \
	src/memory/memory.c \
	src/sync/sync.c \
	src/filesystem/filesystem.c

OBJ = $(SRC:.c=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)
