CC = gcc
CPPFLAGS += -Iinclude
CFLAGS = -Wall -Wextra

SRCS = src/main.c src/shell.c src/system_info.c src/process.c src/ipc.c src/jobs.c
OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)
TARGET = minios
THREAD_FLAGS = -pthread

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(THREAD_FLAGS) $^ $(LDLIBS) -o $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(THREAD_FLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET) uptime_demo fork_demo fork_memory_demo exec_wait_demo pipe_demo

.PHONY: all clean
