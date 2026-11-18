# Member 3 — Worker threads and performance

Copy `include/workers.h`, `src/workers.c`, `tests/worker_benchmark.c`, and this report into the matching paths in the repository.

## Worker behavior

Each pthread takes a job from the queue, marks it running if the job manager supplies that hook, calculates a deterministic 32-bit checksum from the job name, and publishes the job ID, worker ID and checksum. The fixed CPU workload makes it possible to compare worker counts with the same work per job. The statistics mutex protects state and counters. Workers release it before taking jobs, computing checksums or calling the job manager.

The `workers` command can display each worker's state, current job ID when running, successful publications and failed publications. On quit or EOF, stopping the pool closes the queue to new submissions, drains accepted jobs and joins every worker before freeing the pool.

## Integration with the team

The supplied ZIP has a synchronous fork/pipe submission path. The following callback contract is proposed for the asynchronous queue and job manager.

- **Member 4:** Implement `take_job` and `close_queue` with the shared queue. A successful take returns 1 and copies the ID and name. An empty open queue blocks. A closed queue drains queued jobs before returning 0 to every consumer. Closing wakes blocked consumers and any blocked producer.
- **Member 2:** Assign the job ID and store the queued record before making the job available. Implement `mark_running` and the thread-safe `publish_result` adapter. Send the ID, worker ID and checksum through the result pipe. Keep the reader running so the pipe drains even if nobody types `jobs`. Frame writes for multiple producers, and record the job as failed if publishing fails. The callback consumes or copies the result before returning.
- **Member 1:** Add `src/workers.c` to the Makefile and pass `-pthread` at compile and link time. After queue and result-reader initialization, call `workers_start(&pool, 4, &hooks)` and handle its returned error code. Route `workers` to `workers_print(pool, stdout)`. On quit or EOF, call `workers_stop(pool)` while the result reader is still alive. Then close the result writer, let the reader drain to EOF and join it, and release the queue and job-manager resources. Start and stop belong to the shell owner.

Once connected, submit jobs and check that their IDs in `jobs` match their pipe results. Run `workers` before and after submission, then quit with jobs pending to verify the drain and join path.

## Reproduce the benchmark

From the repository root on Ubuntu or WSL:

```sh
gcc -std=c11 -O2 -Wall -Wextra -Werror -pedantic -pthread -Iinclude \
    src/workers.c tests/worker_benchmark.c -o /tmp/minios_test
timeout 10 /tmp/minios_test --workers 8 --empty
for trial in 1 2 3; do
    for workers in 1 2 4 8; do
        timeout 10 /tmp/minios_test --workers "$workers"
    done
done
```

The harness uses a preloaded in-memory job source and a result callback. It checks for missing or duplicate job IDs and reports a checksum total and per-worker counts. Time covers pool startup, processing and shutdown. The benchmark measures the worker pool with this fixture. The integrated semaphore queue and pipe may change the timings.

## Measured results

Ubuntu WSL, GCC 13.3.0 with `-O2`, Intel Core 5 120U with 12 logical CPUs visible. Each non-empty trial processed 2,048 jobs with checksum total `4362827009266`.

| Workers | Trial 1 (s) | Trial 2 (s) | Trial 3 (s) | Median (s) | Speedup vs. 1 worker |
| ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | 0.274857 | 0.256896 | 0.253811 | 0.256896 | 1.00x |
| 2 | 0.126638 | 0.139975 | 0.128383 | 0.128383 | 2.00x |
| 4 | 0.078982 | 0.078082 | 0.076184 | 0.078082 | 3.29x |
| 8 | 0.043573 | 0.043342 | 0.045176 | 0.043573 | 5.90x |

The eight-worker empty run exited with zero results, and a request for nine workers returned a usage error. Increasing the pool to eight workers cut the median from 0.257 s to 0.044 s. Queue and result updates still serialize briefly, and CPU scheduling and shared hardware affect how much work runs in parallel.
