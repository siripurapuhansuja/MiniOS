#ifndef JOBS_H
#define JOBS_H

#include <stddef.h>
#include <stdint.h>

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

#endif
