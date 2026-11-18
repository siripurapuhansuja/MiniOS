# Member 3 — Worker Threads and Performance

## 1. Overview

This module implements a multithreaded worker pool using POSIX threads (`pthread`). Workers retrieve jobs from a shared queue, process them, and publish their results.

The implementation demonstrates concurrent job processing, thread synchronization, worker statistics, and graceful shutdown.

## 2. Worker Behavior

Each worker thread:

1. Retrieves a job from the shared queue.
2. Marks the job as running when the job manager provides the corresponding callback.
3. Calculates a deterministic 32-bit checksum using the job name.
4. Publishes the job ID, worker ID, and checksum.
5. Updates its success or failure statistics.

The worker pool uses a statistics mutex to protect shared worker state and counters.

Workers release the statistics mutex before retrieving jobs, calculating checksums, or calling job-manager callbacks. This avoids holding the statistics lock during potentially blocking operations.

## 3. Worker Statistics

The `workers` command displays worker information, including:

- Worker ID.
- Current worker state.
- Current job ID, when processing a job.
- Number of successful result publications.
- Number of failed result publications.

## 4. Integration with MiniOS

The worker pool is integrated with the other MiniOS modules.

- **Member 1 — Shell and Process Management:** Integrates the worker pool into the application lifecycle and provides shell commands.
- **Member 2 — IPC and Job Management:** Registers jobs, tracks their status, and handles worker-result communication.
- **Member 4 — Queue and Synchronization:** Provides the bounded queue, blocking job retrieval, and queue shutdown.
- **Member 3 — Worker Threads:** Implements worker creation, job processing, result publication, statistics, and worker shutdown.

The worker pool uses callback functions to communicate with the queue and job manager without directly depending on their internal implementations.

## 5. Shutdown Behavior

When MiniOS exits:

1. The worker pool closes the job queue to new submissions.
2. Workers finish jobs that were already accepted into the queue.
3. Workers exit after the queue is closed and drained.
4. The application joins all worker threads.
5. The worker pool releases its allocated resources.

The result reader remains active during worker shutdown so that published results can be consumed.

## 6. Worker Benchmark

The benchmark evaluates worker-pool performance using a preloaded in-memory job source and a result callback.

It processes 2,048 jobs and checks for missing or duplicate job IDs. It also reports the checksum total, per-worker job counts, and elapsed time.

The benchmark measures the worker pool independently of the integrated queue and result pipe. Therefore, integrated MiniOS performance may differ.

### Build the benchmark

From the repository root, run:

```bash
make benchmark
```

### Run the benchmark

Run with eight workers:

```bash
./build/worker_benchmark --workers 8
```

Run with different worker counts:

```bash
for workers in 1 2 4 8
do
    ./build/worker_benchmark --workers "$workers"
done
```

Run the empty-job test:

```bash
./build/worker_benchmark --workers 8 --empty
```

## 7. Measured Results

The following results were recorded using Ubuntu WSL, GCC 13.3.0 with `-O2`, on an Intel Core 5 120U system with 12 logical CPUs visible.

Each non-empty
