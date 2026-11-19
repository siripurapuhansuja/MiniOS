# MiniOS – Operating System Simulation Project

MiniOS is an educational operating system simulation project developed in C for Ubuntu Linux. It demonstrates fundamental operating system concepts, including process management, inter-process communication, synchronisation, job scheduling, and multithreading.

The project uses Linux system calls, POSIX threads, semaphores, and the `/proc` filesystem.

## Project Features

### 1. MiniOS Shell
- Interactive command-line interface.
- Supports commands such as `help`, `system`, `processes`, and `quit`.
- Allows users to execute programs and submit background jobs.
- Displays job and worker information.

### 2. System Information
- Reads system uptime from `/proc/uptime`.
- Reads total memory from `/proc/meminfo`.
- Displays live system information.

### 3. Process Management
- Demonstrates process creation using `fork()`.
- Demonstrates process synchronization using `wait()` and `waitpid()`.
- Executes external programs using `execvp()`.
- Displays running processes using the Linux `/proc` filesystem.
- Demonstrates parent-child process relationships, zombie processes, and orphan processes.

### 4. Inter-Process Communication (IPC)
- Demonstrates communication using `pipe()`, `read()`, and `write()`.
- Implements communication between processes and job-result handling.
- Uses structured worker-result messages.

### 5. Job Queue and Synchronization
- Implements a bounded, thread-safe job queue.
- Uses mutexes and semaphores for synchronization.
- Supports multiple producers and consumers.
- Handles queue shutdown and pending jobs safely.

### 6. Worker Pool
- Implements a multithreaded worker pool using POSIX threads.
- Supports multiple worker threads processing jobs concurrently.
- Tracks worker activity and job results.
- Supports graceful worker shutdown and job draining.

### 7. Race Condition Demonstration
- Demonstrates the effects of concurrent access.
- Compares synchronized and unsynchronized counter operations.
- Uses atomic operations and mutex protection.

## Project Structure

```text
MiniOS/
├── examples/
│   ├── uptime.c
│   ├── fork_demo.c
│   ├── fork_memory_demo.c
│   ├── exec_wait_demo.c
│   └── pipe_demo.c
│
├── include/
│   ├── minios.h
│   ├── runtime.h
│   ├── workers.h
│   ├── sync.h
│   ├── ipc.h
│   └── jobs.h
│
├── src/
│   ├── main.c
│   ├── shell.c
│   ├── system_info.c
│   ├── process.c
│   ├── ipc.c
│   ├── jobs.c
│   ├── workers.c
│   ├── sync.c
│   ├── race.c
│   └── runtime.c
│
├── tests/
│   ├── member4_queue_test.c
│   ├── member4_race_test.c
│   ├── member4_worker_drain_test.c
│   └── worker_benchmark.c
│
├── Makefile
├── README.md
├── MEMBER3.md
└── MEMBER4.md
```

## Requirements

- Ubuntu Linux
- GCC compiler
- GNU Make
- POSIX threads support

## Building the Project

Clone the repository:

```bash
git clone https://github.com/siripurapuhansuja/MiniOS.git
cd MiniOS
```

Build MiniOS and all example programs:

```bash
make
```

This generates the main `minios` executable and the example programs.

## Running MiniOS

Start the shell:

```bash
./minios
```

### Available Shell Commands

| Command | Description |
|---|---|
| `help` | Displays available commands |
| `system` | Displays system information |
| `processes` | Displays running processes |
| `run` | Executes an external program |
| `submit` | Submits a background job |
| `jobs` | Displays job information |
| `workers` | Displays worker information |
| `race` | Runs the race-condition demonstration |
| `quit` | Exits MiniOS |

## Running Example Programs

Build all examples:

```bash
make examples
```

Run individual demonstrations:

```bash
./uptime_demo
./fork_demo
./fork_memory_demo
./exec_wait_demo
./pipe_demo
```


## Testing

Build and execute the automated test suite:

```bash
make test
```

The test suite includes:

- Queue FIFO ordering and shutdown.
- Multiple producers and consumers.
- Race-condition demonstrations.
- Worker shutdown and pending-job draining.
- Member 5 integration testing of the MiniOS shell, system information, process listing, job submission, worker status, race demonstration, and graceful shutdown.

Run the Member 5 integration test independently:

```bash
bash tests/test_m5.sh
```


## Worker Benchmark

Build the worker benchmark:

```bash
make benchmark
```

Run the benchmark with four workers:

```bash
./build/worker_benchmark --workers 4
```

The benchmark measures worker-pool performance with different worker counts.

## Cleaning Generated Files

Remove compiled executables, object files, dependency files, and build output:

```bash
make clean
```

## Project Objective

The objective of MiniOS is to provide practical experience with fundamental operating system concepts through a modular C implementation. It demonstrates how processes, threads, synchronisation mechanisms, communication channels, and job queues work together in a simplified operating system environment.

---

**Technology Stack:** C, GCC, Linux, POSIX Threads, Semaphores, Mutexes, Linux `/proc` Filesystem, and GNU Make.
