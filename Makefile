CC = gcc
CFLAGS = -Iinclude -Wall -Wextra

SRC_DIR = src
INC_DIR = include

SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/shell.c $(SRC_DIR)/system_info.c
OBJS = $(SRCS:.c=.o)
TARGET = minios

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SRC_DIR)/*.o $(TARGET) uptime_demo fork_demo fork_memory_demo exec_wait_demo pipe_demo

.PHONY: all clean
