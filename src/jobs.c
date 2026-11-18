
#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include "jobs.h"
#include "ipc.h"

#define MAX_JOBS 20

static Job jobs[MAX_JOBS];
static int job_count = 0;

static pthread_mutex_t jobs_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t pipe_write_lock = PTHREAD_MUTEX_INITIALIZER;

static pthread_t reader_thread;
static int result_pipe[2] = {-1, -1};
static int reader_started = 0;

/* --------------------------------------------------
   Display job information
   -------------------------------------------------- */

void list_jobs(void) {
    pthread_mutex_lock(&jobs_lock);

    if (job_count == 0) {
        printf("No jobs submitted yet.\n");
        pthread_mutex_unlock(&jobs_lock);
        return;
    }

    printf("\n%-5s %-20s %-12s\n",
           "ID", "JOB NAME", "STATUS");

    for (int i = 0; i < job_count; i++) {
        const char *status;

        switch (jobs[i].status) {
            case JOB_QUEUED:
                status = "Queued";
                break;

            case JOB_RUNNING:
                status = "Running";
                break;

            case JOB_COMPLETED:
                status = "Completed";
                break;

            case JOB_FAILED:
                status = "Failed";
                break;

            default:
                status = "Unknown";
                break;
        }

        printf("%-5d %-20s %-12s\n",
               jobs[i].id,
               jobs[i].name,
               status);
    }

    pthread_mutex_unlock(&jobs_lock);
}

/* --------------------------------------------------
   Background pipe reader
   -------------------------------------------------- */

static void *result_reader(void *arg) {
    (void)arg;

    WorkerResult result;

    for (;;) {
        int rc = receive_worker_result(result_pipe[0],
                                       &result);

        if (rc == 0) {
            break;
        }

        if (rc < 0) {
            perror("receive_worker_result");
            break;
        }

        pthread_mutex_lock(&jobs_lock);

        for (int i = 0; i < job_count; i++) {
            if (jobs[i].id == result.job_id) {
                if (jobs[i].status != JOB_COMPLETED &&
                    jobs[i].status != JOB_FAILED) {

                    jobs[i].worker_id = result.worker_id;
                    jobs[i].checksum = result.checksum;
                    jobs[i].status = JOB_COMPLETED;
                }

                break;
            }
        }

        pthread_mutex_unlock(&jobs_lock);
    }

    return NULL;
}

/* --------------------------------------------------
   Start background reader
   -------------------------------------------------- */

int jobs_start(void) {
    if (reader_started) {
        return -1;
    }

    if (pipe2(result_pipe, O_CLOEXEC) == -1) {
        perror("pipe");
        return -1;
    }

    int rc = pthread_create(&reader_thread,
                            NULL,
                            result_reader,
                            NULL);

    if (rc != 0) {
        errno = rc;
        perror("pthread_create");

        close(result_pipe[0]);
        close(result_pipe[1]);

        result_pipe[0] = -1;
        result_pipe[1] = -1;

        return -1;
    }

    reader_started = 1;

    return 0;
}

/* --------------------------------------------------
   Register a new job
   -------------------------------------------------- */

int jobs_register(const char *name) {
    if (name == NULL || *name == '\0') {
        return -1;
    }

    pthread_mutex_lock(&jobs_lock);

    if (job_count >= MAX_JOBS) {
        pthread_mutex_unlock(&jobs_lock);
        return -1;
    }

    Job *job = &jobs[job_count];

    job->id = job_count + 1;

    snprintf(job->name, sizeof(job->name),
             "%s", name);

    job->status = JOB_QUEUED;
    job->worker_id = 0;
    job->checksum = 0;

    int id = job->id;

    job_count++;

    pthread_mutex_unlock(&jobs_lock);

    return id;
}

/* --------------------------------------------------
   Mark job as running
   -------------------------------------------------- */

void jobs_mark_running(int job_id) {
    pthread_mutex_lock(&jobs_lock);

    for (int i = 0; i < job_count; i++) {
        if (jobs[i].id == job_id) {
            if (jobs[i].status == JOB_QUEUED) {
                jobs[i].status = JOB_RUNNING;
            }
            break;
        }
    }

    pthread_mutex_unlock(&jobs_lock);
}
/* Mark a job as failed. */
void jobs_mark_failed(int job_id) {
    pthread_mutex_lock(&jobs_lock);

    for (int i = 0; i < job_count; i++) {
        if (jobs[i].id == job_id) {
            if (jobs[i].status != JOB_COMPLETED) {
                jobs[i].status = JOB_FAILED;
            }
            break;
        }
    }

    pthread_mutex_unlock(&jobs_lock);
}
/* --------------------------------------------------
   Publish worker result through pipe
   -------------------------------------------------- */

int jobs_publish_result(const WorkerResult *result) {
    if (result == NULL || !reader_started) {
        errno = EINVAL;
        return -1;
    }

    pthread_mutex_lock(&pipe_write_lock);

    int rc = send_worker_result(result_pipe[1], result);

    pthread_mutex_unlock(&pipe_write_lock);

    return rc;
}

/* --------------------------------------------------
   Stop background reader
   -------------------------------------------------- */

void jobs_stop(void) {
    if (!reader_started) {
        return;
    }

    /*
     * All worker threads must finish publishing
     * before this function is called.
     */

    close(result_pipe[1]);
    result_pipe[1] = -1;

    pthread_join(reader_thread, NULL);

    close(result_pipe[0]);
    result_pipe[0] = -1;

    reader_started = 0;
}

