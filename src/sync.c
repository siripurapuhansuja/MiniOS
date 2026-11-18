#include "sync.h"

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>

struct JobQueue {
    WorkerJob *jobs;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t count;
    size_t waiting_pushers;
    size_t waiting_poppers;
    int closed;
    pthread_mutex_t lock;
    sem_t empty;
    sem_t full;
};

int queue_create(JobQueue **out, size_t capacity)
{
    if (out == NULL)
        return EINVAL;

    *out = NULL;
    if (capacity == 0 || capacity != (unsigned int)capacity)
        return EINVAL;

    JobQueue *queue = calloc(1, sizeof(*queue));
    if (queue == NULL)
        return ENOMEM;

    queue->jobs = calloc(capacity, sizeof(*queue->jobs));
    if (queue->jobs == NULL) {
        free(queue);
        return ENOMEM;
    }
    queue->capacity = capacity;

    int error = pthread_mutex_init(&queue->lock, NULL);
    if (error != 0)
        goto free_queue;

    if (sem_init(&queue->empty, 0, (unsigned int)capacity) == -1) {
        error = errno;
        goto destroy_lock;
    }

    if (sem_init(&queue->full, 0, 0) == -1) {
        error = errno;
        goto destroy_empty;
    }

    *out = queue;
    return 0;

destroy_empty:
    sem_destroy(&queue->empty);
destroy_lock:
    pthread_mutex_destroy(&queue->lock);
free_queue:
    free(queue->jobs);
    free(queue);
    return error;
}

/* Retry if a signal interrupts the semaphore wait. */
static int wait_for_slot(sem_t *semaphore)
{
    while (sem_wait(semaphore) == -1) {
        if (errno != EINTR)
            return errno;
    }
    return 0;
}

int queue_push(JobQueue *queue, const WorkerJob *job)
{
    pthread_mutex_lock(&queue->lock);
    if (queue->closed) {
        pthread_mutex_unlock(&queue->lock);
        return ECANCELED;
    }

    queue->waiting_pushers++;
    pthread_mutex_unlock(&queue->lock);

    int error = wait_for_slot(&queue->empty);

    pthread_mutex_lock(&queue->lock);
    queue->waiting_pushers--;

    if (error == 0 && queue->closed) {
        if (queue->waiting_pushers > 0)
            sem_post(&queue->empty);
        error = ECANCELED;
    }

    if (error == 0) {
        queue->jobs[queue->tail] = *job;
        queue->tail = (queue->tail + 1) % queue->capacity;
        queue->count++;
        sem_post(&queue->full);
    }

    pthread_mutex_unlock(&queue->lock);
    return error;
}

int queue_pop(void *context, WorkerJob *job)
{
    JobQueue *queue = context;

    pthread_mutex_lock(&queue->lock);
    if (queue->closed && queue->count == 0) {
        pthread_mutex_unlock(&queue->lock);
        return 0;
    }

    queue->waiting_poppers++;
    pthread_mutex_unlock(&queue->lock);

    int error = wait_for_slot(&queue->full);

    pthread_mutex_lock(&queue->lock);
    queue->waiting_poppers--;

    if (error != 0) {
        pthread_mutex_unlock(&queue->lock);
        return -error;
    }

    if (queue->count == 0) {
        if (queue->waiting_poppers > 0)
            sem_post(&queue->full);
        pthread_mutex_unlock(&queue->lock);
        return 0;
    }

    *job = queue->jobs[queue->head];
    queue->head = (queue->head + 1) % queue->capacity;
    queue->count--;

    if (!queue->closed)
        sem_post(&queue->empty);

    pthread_mutex_unlock(&queue->lock);
    return 1;
}

void queue_close(void *context)
{
    JobQueue *queue = context;

    pthread_mutex_lock(&queue->lock);
    if (!queue->closed) {
        queue->closed = 1;

        if (queue->waiting_pushers > 0)
            sem_post(&queue->empty);

        if (queue->waiting_poppers > 0)
            sem_post(&queue->full);
    }
    pthread_mutex_unlock(&queue->lock);
}

void queue_destroy(JobQueue *queue)
{
    if (queue == NULL)
        return;

    sem_destroy(&queue->full);
    sem_destroy(&queue->empty);
    pthread_mutex_destroy(&queue->lock);
    free(queue->jobs);
    free(queue);
}