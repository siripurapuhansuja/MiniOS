
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "jobs.h"
#include "ipc.h"

#define MAX_JOBS 20

static Job jobs[MAX_JOBS];
static int job_count = 0;

void submit_job(const char *name) {
    if (name == NULL || *name == '\0') {
        printf("Usage: submit <job_name>\n");
        return;
    }

    if (job_count >= MAX_JOBS) {
        printf("Job list is full.\n");
        return;
    }

    int fd[2];

    if (pipe(fd) == -1) {
        perror("pipe");
        return;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        return;
    }

    if (pid == 0) {
        close(fd[0]);

        char result[128];

        snprintf(result, sizeof(result),
                 "Job '%s' completed", name);

        if (send_result(fd[1], result) < 0) {
            perror("send_result");
            close(fd[1]);
            _exit(1);
        }

        close(fd[1]);
        _exit(0);
    }

    close(fd[1]);

    Job *job = &jobs[job_count];

    job->id = job_count + 1;

    snprintf(job->name, sizeof(job->name),
             "%s", name);

    job->status = JOB_RUNNING;
    job->worker_id = 0;
    job->checksum = 0;

    char result[128];

    ssize_t n = receive_result(fd[0],
                               result,
                               sizeof(result));

    close(fd[0]);

    int status;

    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return;
    }

    if (n < 0 || !WIFEXITED(status) ||
        WEXITSTATUS(status) != 0) {
        printf("Job execution failed.\n");
        return;
    }

    job->status = JOB_COMPLETED;
    job_count++;

    printf("Submitted Job %d: %s\n",
           job->id, job->name);

    printf("Manager received: %s\n", result);
}

void list_jobs(void) {
    if (job_count == 0) {
        printf("No jobs submitted yet.\n");
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
}
