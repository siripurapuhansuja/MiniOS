
CC = gcc

CPPFLAGS += -Iinclude
CFLAGS = -Wall -Wextra
THREAD_FLAGS = -pthread

SRCS = src/main.c src/shell.c src/system_info.c src/process.c \
       src/ipc.c src/jobs.c src/workers.c src/sync.c \
       src/race.c src/runtime.c

OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)

TARGET = minios
BUILD_DIR = build

EXAMPLES = uptime_demo fork_demo fork_memory_demo exec_wait_demo pipe_demo

QUEUE_TEST = $(BUILD_DIR)/member4_queue_test
RACE_TEST = $(BUILD_DIR)/member4_race_test
DRAIN_TEST = $(BUILD_DIR)/member4_worker_drain_test
BENCHMARK = $(BUILD_DIR)/worker_benchmark

all: $(TARGET) examples

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(THREAD_FLAGS) $^ $(LDLIBS) -o $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(THREAD_FLAGS) -MMD -MP -c $< -o $@

examples: $(EXAMPLES)

uptime_demo: examples/uptime.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

fork_demo: examples/fork_demo.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

fork_memory_demo: examples/fork_memory_demo.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

exec_wait_demo: examples/exec_wait_demo.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

pipe_demo: examples/pipe_demo.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(QUEUE_TEST): tests/member4_queue_test.c src/sync.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(THREAD_FLAGS) $^ -o $@

$(RACE_TEST): tests/member4_race_test.c src/race.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(THREAD_FLAGS) $^ -o $@

$(DRAIN_TEST): tests/member4_worker_drain_test.c src/workers.c src/sync.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(THREAD_FLAGS) $^ -o $@

$(BENCHMARK): tests/worker_benchmark.c src/workers.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(THREAD_FLAGS) $^ -o $@

test: $(QUEUE_TEST) $(RACE_TEST) $(DRAIN_TEST)
	./$(QUEUE_TEST)
	./$(RACE_TEST)
	./$(DRAIN_TEST)
	bash tests/test_m5.sh

benchmark: $(BENCHMARK)
	@echo "Run with: ./$(BENCHMARK) --workers 4"

-include $(DEPS)

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET) $(EXAMPLES)
	rm -rf $(BUILD_DIR)

.PHONY: all examples test benchmark clean

