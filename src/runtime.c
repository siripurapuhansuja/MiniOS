#include <stdio.h>

#include "runtime.h"
#include "jobs.h"
#include "workers.h"
#include "sync.h"

typedef struct {
    JobQueue *queue;
    WorkerPool *workers;
} Runtime;

static Runtime runtime = {0};

/* Called when a worker starts processing a job. */
static void mark_running(void *context, int job_id)
{
    (void)context;
    jobs_mark_running(job_id);
}

/* Called when a worker finishes processing a job. */
static int publish_result(void *context, const WorkerResult *result)
{
    (void)context;

    int status = jobs_publish_result(result);

    if (status != 0) {
        jobs_mark_failed(result->job_id);
    }

    return status;
}
int runtime_start(void)
{
    int status = queue_create(&runtime.queue, 4);

    if (status != 0) {
        fprintf(stderr, "Failed to create job queue.\n");
        return -1;
    }

    if (jobs_start() != 0) {
        fprintf(stderr, "Failed to start result reader.\n");
        queue_destroy(runtime.queue);
        runtime.queue = NULL;
        return -1;
    }

    WorkerHooks hooks = {
        .context = runtime.queue,
        .take_job = queue_pop,
        .mark_running = mark_running,
        .publish_result = publish_result,
        .close_queue = queue_close
    };

    status = workers_start(&runtime.workers, 2, &hooks);

    if (status != 0) {
        fprintf(stderr, "Failed to start worker pool.\n");
        jobs_stop();
        queue_destroy(runtime.queue);
        runtime.queue = NULL;
        return -1;
    }

    return 0;
}
void runtime_submit(const char *name)
{
    if (name == NULL || *name == '\0') {
        printf("Usage: submit <job_name>\n");
        return;
    }

    if (runtime.queue == NULL || runtime.workers == NULL) {
        fprintf(stderr, "Job system is not running.\n");
        return;
    }

    int id = jobs_register(name);

    if (id < 0) {
        printf("Job list is full or job is invalid.\n");
        return;
    }

    WorkerJob job = { .id = id };
    snprintf(job.name, sizeof(job.name), "%s", name);

    int status = queue_push(runtime.queue, &job);

    if (status != 0) {
        jobs_mark_failed(id);
        fprintf(stderr, "Failed to queue Job %d.\n", id);
        return;
    }

    printf("Submitted Job %d: %s\n", id, job.name);
}
void runtime_stop(void)
{
    if (runtime.workers != NULL) {
        workers_stop(runtime.workers);
        runtime.workers = NULL;
    }

    jobs_stop();

    if (runtime.queue != NULL) {
        queue_destroy(runtime.queue);
        runtime.queue = NULL;
    }
}
void runtime_print_workers(void)
{
    if (runtime.workers == NULL) {
        fprintf(stderr, "Worker pool is not running.\n");
        return;
    }

    workers_print(runtime.workers, stdout);
}
