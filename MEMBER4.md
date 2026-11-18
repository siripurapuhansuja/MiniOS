# Member 4 — Queue and Synchronization

## 1. Overview

This module implements a bounded, thread-safe job queue for MiniOS.

The queue allows multiple producer and consumer threads to exchange jobs safely. It uses POSIX synchronization mechanisms, including mutexes and semaphores.

The queue has a capacity of two slots in the queue tests.

## 2. Queue Implementation

The queue supports the following operations:

- `queue_create()` — Creates and initializes the queue.
- `queue_push()` — Adds a job to the queue.
- `queue_pop()` — Retrieves a job from the queue.
- `queue_close()` — Closes the queue and wakes blocked threads.
- `queue_destroy()` — Releases queue resources.

### Queue Behavior

1. Jobs are retrieved in FIFO (First In, First Out) order.
2. Producers wait when the queue is full.
3. Consumers wait when the queue is empty.
4. Closing the queue prevents new jobs from being accepted.
5. Already accepted jobs can still be retrieved.
6. Blocked producers and consumers are awakened during shutdown.
7. The queue is destroyed only after threads using it have exited.

## 3. Queue Testing

The queue implementation was tested using the following scenarios:

| Test | Description |
|---|---|
| FIFO ordering | Verifies that jobs are retrieved in insertion order. |
| Producer blocking | Verifies that a producer waits when the queue is full. |
| Idle consumer shutdown | Verifies that blocked consumers wake when the queue closes. |
| Full queue shutdown | Verifies that accepted jobs remain available and additional submissions are rejected. |

The two-slot queue passed these four test scenarios.

### Run the queue test

From the repository root:

```bash
make test
```

The queue test can also be compiled and executed independently:

```bash
gcc -std=c11 -Wall -Wextra -Werror -pthread -Iinclude \
    src/sync.c tests/member4_queue_test.c \
    -o /tmp/minios-member4-test

timeout 5s /tmp/minios-member4-test
```

The test should display four lines beginning with `PASS`.

## 4. Race Condition Experiment

The race-condition experiment demonstrates what happens when multiple threads access and modify shared data.

Four threads each attempt 100,000 increments.

The expected final count is:

```text
4 × 100000 = 400000
```

### Recorded Results

| Trial | Expected | Without Mutex | With Mutex |
|---|---:|---:|---:|
| 1 | 400000 | 380817 | 400000 |
| 2 | 400000 | 255769 | 400000 |
| 3 | 400000 | 313745 | 400000 |

Without proper synchronization, concurrent updates can overwrite one another, resulting in lost increments.

With mutex protection, only one thread can execute the protected increment operation at a time.

The experiment uses atomic operations for the unprotected counter so that the demonstration avoids an undefined C data race while still showing lost updates.

### Run the race test

```bash
make test
```

The race test executes three trials.

## 5. Integration with MiniOS

The queue and synchronization module integrates with the other project modules.

- **Member 1 — Shell and Process Management:** Integrates the queue into the application lifecycle.
- **Member 2 — IPC and Job Management:** Submits jobs to the queue and tracks their IDs and statuses.
- **Member 3 — Worker Threads:** Uses queue operations to retrieve and process jobs.
- **Member 4 — Queue and Synchronization:** Implements thread-safe queue operations and shutdown behavior.

The worker pool uses `queue_pop()` as its job-retrieval callback and `queue_close()` as its shutdown callback.

The queue must not be destroyed until all producer and consumer threads have stopped using it.

## 6. Worker Shutdown and Queue Draining

The queue supports graceful shutdown.

When the queue closes:

1. New submissions are rejected.
2. Blocked producers are awakened.
3. Consumers can retrieve jobs that were already accepted.
4. Consumers exit after the queue becomes empty.
5. Queue resources are released after all associated threads have exited.

The worker-drain test verifies that accepted jobs are completed during worker shutdown.

## 7. Automated Tests

The repository includes the following tests:

- `tests/member4_queue_test.c` — Tests queue behavior and shutdown.
- `tests/member4_race_test.c` — Runs the race-condition experiment.
- `tests/member4_worker_drain_test.c` — Tests worker shutdown and job draining.
- `tests/worker_benchmark.c` — Measures worker-pool performance.

Build and run the automated tests:

```bash
make test
```

## 8. Conclusion

The queue and synchronization module provides safe communication between producer and consumer threads. It demonstrates FIFO ordering, blocking operations, mutex and semaphore synchronization, race-condition prevention, and graceful shutdown.

These mechanisms form the foundation for reliable concurrent job processing in MiniOS.
