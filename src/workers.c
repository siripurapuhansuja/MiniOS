#include "workers.h"

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>

typedef enum {
    WAITING,
    RUNNING,
    STOPPED
} WorkerState;

typedef struct {
    WorkerState state;
    int job_id;
    size_t completed;
    size_t failed;
} WorkerStats;

typedef struct {
    WorkerPool *pool;
    pthread_t thread;
    size_t id;
    WorkerStats stats;
} Worker;

struct WorkerPool {
    WorkerHooks hooks;
    pthread_mutex_t lock;
    int started;
    size_t count;
    Worker workers[WORKERS_MAX];
};

static uint32_t job_checksum(const WorkerJob *job) {
    /* Fixed CPU work for every job, seeded with its name. Unsigned
       arithmetic deliberately wraps at 32 bits. */
    uint32_t value = UINT32_C(2166136261);
    for (size_t i = 0; i < sizeof(job->name) && job->name[i]; i++) {
        value ^= (unsigned char)job->name[i];
        value *= UINT32_C(16777619);
    }
    for (unsigned i = 0; i < 100000; i++) {
        value ^= value << 13;
        value ^= value >> 17;
        value ^= value << 5;
    }
    return value;
}

static void *worker_main(void *arg) {
    Worker *worker = arg;
    WorkerPool *pool = worker->pool;
    WorkerHooks *hooks = &pool->hooks;

    /* Wait for startup to commit before touching the queue. */
    pthread_mutex_lock(&pool->lock);
    int started = pool->started;
    pthread_mutex_unlock(&pool->lock);
    if (!started) {
        return NULL;
    }

    WorkerJob job;
    while (hooks->take_job(hooks->context, &job) == 1) {
        pthread_mutex_lock(&pool->lock);
        worker->stats.state = RUNNING;
        worker->stats.job_id = job.id;
        pthread_mutex_unlock(&pool->lock);

        if (hooks->mark_running != NULL) {
            hooks->mark_running(hooks->context, job.id);
        }
        WorkerResult result = {
            .job_id = job.id,
            .worker_id = worker->id,
            .checksum = job_checksum(&job)
        };
        int published = hooks->publish_result(hooks->context, &result);

        pthread_mutex_lock(&pool->lock);
        if (published == 0) {
            worker->stats.completed++;
        } else {
            worker->stats.failed++;
        }
        worker->stats.state = WAITING;
        pthread_mutex_unlock(&pool->lock);
    }

    pthread_mutex_lock(&pool->lock);
    worker->stats.state = STOPPED;
    pthread_mutex_unlock(&pool->lock);
    return NULL;
}

static void join_and_free(WorkerPool *pool) {
    for (size_t i = 0; i < pool->count; i++) {
        pthread_join(pool->workers[i].thread, NULL);
    }
    pthread_mutex_destroy(&pool->lock);
    free(pool);
}

int workers_start(WorkerPool **out, size_t count, const WorkerHooks *hooks) {
    if (out == NULL) {
        return EINVAL;
    }
    *out = NULL;
    if (count == 0 || count > WORKERS_MAX || hooks == NULL ||
        hooks->take_job == NULL || hooks->publish_result == NULL ||
        hooks->close_queue == NULL) {
        return EINVAL;
    }

    WorkerPool *pool = calloc(1, sizeof(*pool));
    if (pool == NULL) {
        return ENOMEM;
    }
    int error = pthread_mutex_init(&pool->lock, NULL);
    if (error != 0) {
        free(pool);
        return error;
    }
    pool->hooks = *hooks;

    pthread_mutex_lock(&pool->lock);
    for (size_t i = 0; i < count; i++) {
        Worker *worker = &pool->workers[i];
        worker->pool = pool;
        worker->id = i + 1;
        error = pthread_create(&worker->thread, NULL, worker_main, worker);
        if (error != 0) {
            pthread_mutex_unlock(&pool->lock);
            join_and_free(pool);
            return error;
        }
        pool->count++;
    }
    pool->started = 1;
    pthread_mutex_unlock(&pool->lock);

    *out = pool;
    return 0;
}

void workers_print(WorkerPool *pool, FILE *out) {
    if (pool == NULL) {
        fputs("Worker pool is inactive.\n", out);
        return;
    }

    WorkerStats snapshot[WORKERS_MAX];
    pthread_mutex_lock(&pool->lock);
    for (size_t i = 0; i < pool->count; i++) {
        snapshot[i] = pool->workers[i].stats;
    }
    pthread_mutex_unlock(&pool->lock);

    fprintf(out, "%-8s %-9s %-8s %-10s %s\n",
            "WORKER", "STATE", "JOB", "COMPLETED", "FAILED");
    for (size_t i = 0; i < pool->count; i++) {
        WorkerStats *stats = &snapshot[i];
        const char *states[] = {"Waiting", "Running", "Stopped"};
        fprintf(out, "%-8zu %-9s ", i + 1, states[stats->state]);
        if (stats->state == RUNNING) {
            fprintf(out, "%-8d ", stats->job_id);
        } else {
            fprintf(out, "%-8s ", "-");
        }
        fprintf(out, "%-10zu %zu\n", stats->completed, stats->failed);
    }
}

void workers_stop(WorkerPool *pool) {
    if (pool != NULL) {
        pool->hooks.close_queue(pool->hooks.context);
        join_and_free(pool);
    }
}
