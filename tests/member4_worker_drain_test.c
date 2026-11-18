#include <assert.h>
#include <semaphore.h>
#include <stdio.h>
#include "sync.h"

static sem_t allow_first_result;
static int results;

static int record_result(void *context, const WorkerResult *result)
{
    (void)context;
    if (result->job_id == 1)
        assert(sem_wait(&allow_first_result) == 0);

    results++;
    assert(result->job_id == results);
    return 0;
}

static void close_and_release(void *context)
{
    queue_close(context);
    assert(sem_post(&allow_first_result) == 0);
}

int main(void)
{
    JobQueue *queue;
    WorkerPool *pool;
    assert(sem_init(&allow_first_result, 0, 0) == 0);
    assert(queue_create(&queue, 2) == 0);

    WorkerHooks hooks = {
        .context = queue,
        .take_job = queue_pop,
        .publish_result = record_result,
        .close_queue = close_and_release
    };

    assert(workers_start(&pool, 1, &hooks) == 0);
    for (int id = 1; id <= 3; id++) {
        WorkerJob job = {.id = id, .name = "shutdown-test"};
        assert(queue_push(queue, &job) == 0);
    }

    workers_stop(pool);
    assert(results == 3);
    queue_destroy(queue);
    assert(sem_destroy(&allow_first_result) == 0);
    puts("PASS: The worker finished both queued jobs after close.");
}
