#ifndef JOBS_H
#define JOBS_H

#include <stddef.h>
#include <stdint.h>
#include "workers.h"


#define JOB_NAME_SIZE 50

typedef enum {
    JOB_QUEUED,
    JOB_RUNNING,
    JOB_COMPLETED,
    JOB_FAILED
} JobStatus;

typedef struct {
    int id;
    char name[JOB_NAME_SIZE];
    JobStatus status;
    size_t worker_id;
    uint32_t checksum;
} Job;

void submit_job(const char *name);
void list_jobs(void);

/* Start the background result reader. */
int jobs_start(void);

/* Stop the reader after all result producers have finished. */
void jobs_stop(void);

/* Register a job and return its ID, or -1 on failure. */
int jobs_register(const char *name);

/* Update a job when a worker starts processing it. */
void jobs_mark_running(int job_id);
void jobs_mark_failed(int job_id);

/* Publish a worker result through the pipe. */
int jobs_publish_result(const WorkerResult *result);

#endif
