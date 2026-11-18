#ifndef MINIOS_SYNC_H
#define MINIOS_SYNC_H

#include "workers.h"

typedef struct JobQueue JobQueue;

int queue_create(JobQueue **out, size_t capacity);
int queue_push(JobQueue *queue, const WorkerJob *job);
int queue_pop(void *context, WorkerJob *job);
void queue_close(void *context);
void queue_destroy(JobQueue *queue);

#endif