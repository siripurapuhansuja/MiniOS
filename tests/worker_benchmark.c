#define _POSIX_C_SOURCE 200809L

#include "workers.h"

#include <inttypes.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define JOB_COUNT 2048

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t closed;
    int closing;
    int next_job;
    int job_count;
    unsigned char seen[JOB_COUNT];
    size_t per_worker[WORKERS_MAX];
    uint64_t checksum_sum;
} Benchmark;

static int take_job(void *context, WorkerJob *job) {
    Benchmark *bench = context;
    pthread_mutex_lock(&bench->lock);
    while (bench->next_job == bench->job_count && !bench->closing) {
        pthread_cond_wait(&bench->closed, &bench->lock);
    }
    if (bench->next_job == bench->job_count) {
        pthread_mutex_unlock(&bench->lock);
        return 0;
    }
    job->id = ++bench->next_job;
    pthread_mutex_unlock(&bench->lock);
    snprintf(job->name, sizeof(job->name), "job-%d", job->id);
    return 1;
}

static int publish_result(void *context, const WorkerResult *result) {
    Benchmark *bench = context;
    pthread_mutex_lock(&bench->lock);
    if (result->job_id < 1 || result->job_id > bench->job_count ||
        result->worker_id < 1 || result->worker_id > WORKERS_MAX ||
        bench->seen[result->job_id - 1]) {
        pthread_mutex_unlock(&bench->lock);
        return -1;
    }
    bench->seen[result->job_id - 1] = 1;
    bench->per_worker[result->worker_id - 1]++;
    bench->checksum_sum += result->checksum;
    pthread_mutex_unlock(&bench->lock);
    return 0;
}

static void close_queue(void *context) {
    Benchmark *bench = context;
    pthread_mutex_lock(&bench->lock);
    bench->closing = 1;
    pthread_cond_broadcast(&bench->closed);
    pthread_mutex_unlock(&bench->lock);
}

int main(int argc, char **argv) {
    long count = 0;
    if (argc == 3 || argc == 4) {
        char *end;
        count = strtol(argv[2], &end, 10);
        if (argv[2] == end || *end || strcmp(argv[1], "--workers") != 0 ||
            (argc == 4 && strcmp(argv[3], "--empty") != 0)) {
            count = 0;
        }
    }
    if (count < 1 || count > WORKERS_MAX) {
        fprintf(stderr, "Usage: %s --workers 1..%d [--empty]\n",
                argv[0], WORKERS_MAX);
        return EXIT_FAILURE;
    }

    Benchmark bench = {
        .lock = PTHREAD_MUTEX_INITIALIZER,
        .closed = PTHREAD_COND_INITIALIZER,
        .job_count = argc == 4 ? 0 : JOB_COUNT
    };
    WorkerHooks hooks = {
        .context = &bench,
        .take_job = take_job,
        .publish_result = publish_result,
        .close_queue = close_queue
    };

    WorkerPool *pool;
    struct timespec begin, end;
    if (clock_gettime(CLOCK_MONOTONIC, &begin) != 0) {
        perror("clock_gettime");
        return EXIT_FAILURE;
    }
    int error = workers_start(&pool, (size_t)count, &hooks);
    if (error != 0) {
        fprintf(stderr, "workers_start: %s\n", strerror(error));
        return EXIT_FAILURE;
    }
    workers_stop(pool);
    if (clock_gettime(CLOCK_MONOTONIC, &end) != 0) {
        perror("clock_gettime");
        return EXIT_FAILURE;
    }
    pthread_cond_destroy(&bench.closed);
    pthread_mutex_destroy(&bench.lock);

    size_t completed = 0;
    for (size_t i = 0; i < (size_t)count; i++) {
        completed += bench.per_worker[i];
    }
    if (completed != (size_t)bench.job_count) {
        fprintf(stderr, "Expected %d unique results, got %zu\n",
                bench.job_count, completed);
        return EXIT_FAILURE;
    }

    double seconds = (double)(end.tv_sec - begin.tv_sec) +
                     (double)(end.tv_nsec - begin.tv_nsec) / 1e9;
    printf("workers=%ld jobs=%zu seconds=%.6f checksum_sum=%" PRIu64 "\n",
           count, completed, seconds, bench.checksum_sum);
    for (size_t i = 0; i < (size_t)count; i++) {
        printf("worker %zu completed %zu\n", i + 1, bench.per_worker[i]);
    }
    return EXIT_SUCCESS;
}
