# Member 4 — Queue and synchronization

The two-slot queue passed tests for FIFO order, three jobs going
through two slots, shutdown with an idle worker, and shutdown with
a full queue. Closing the full queue kept both accepted jobs and
rejected the third.

## Race experiment

Four threads each attempted 100,000 increments.

| Trial | Expected | Without mutex | With mutex |
| --- | ---: | ---: | ---: |
| 1 | 400000 | 380817 | 400000 |
| 2 | 400000 | 255769 | 400000 |
| 3 | 400000 | 313745 | 400000 |

Without the mutex, threads can read the same value and overwrite
each other's increments. The mutex protects the complete increment.

## Team integration

- Member 1: Add src/sync.c and src/race.c to the Makefile.
  Call race_run() when the user types race.
- Member 2: Call queue_push with the job ID and name. It waits
  when the queue is full and returns ECANCELED if the queue closes.
- Member 3: Use queue_pop as take_job and queue_close as close_queue.
  Call queue_destroy only after all threads using the queue have exited.

## Run the queue tests

From the MiniOS folder in Ubuntu or WSL, run:

```bash
gcc -std=c11 -Wall -Wextra -Werror -pthread -Iinclude \
    src/sync.c tests/member4_queue_test.c -o /tmp/minios-member4-test \
    && timeout 5s /tmp/minios-member4-test
```

Expect four lines starting with PASS.
