#ifndef WORKERS_H
#define WORKERS_H

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

#define WORKERS_MAX 8
#define WORKER_JOB_NAME_SIZE 50

typedef struct {
    int id;
    char name[WORKER_JOB_NAME_SIZE];
} WorkerJob;

typedef struct {
    int job_id;
    size_t worker_id;
    uint32_t checksum;
} WorkerResult;

typedef struct {
    void *context;

    /* Block until a job is available. Fill job and return 1, or return 0
       once the queue is closed and drained. Copy the job before unlocking. */
    int (*take_job)(void *context, WorkerJob *job);

    /* Optional. Update job status to Running. */
    void (*mark_running)(void *context, int job_id);

    /* Publish or record failure before returning. Return 0 on success.
       Called concurrently. The result pointer is valid only during the call. */
    int (*publish_result)(void *context, const WorkerResult *result);

    /* Stop accepting jobs and wake all consumers. Preserve queued jobs. */
    void (*close_queue)(void *context);
} WorkerHooks;

typedef struct WorkerPool WorkerPool;

/* Returns a pthread-style error code. On failure *out is NULL and the
   queue is untouched. The hook context must outlive the pool. */
int workers_start(WorkerPool **out, size_t count, const WorkerHooks *hooks);

/* Print a consistent snapshot. Call from the shell while the pool is live. */
void workers_print(WorkerPool *pool, FILE *out);

/* Owner only. Close the queue, drain accepted jobs, join threads and free
   the pool. Keep the result consumer alive until this function returns. */
void workers_stop(WorkerPool *pool);

#endif
