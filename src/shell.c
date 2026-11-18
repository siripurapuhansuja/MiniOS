#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jobs.h"
#include "minios.h"
#include "system_info.h"
#include "sync.h"

static const char separators[] = " \t\r\n\v\f";


enum { WORKER_COUNT = 4, QUEUE_CAPACITY = 5 };

static void mark_running(void *context, int job_id)
{
    (void)context;
    jobs_mark_running(job_id);
}

static int publish_result(void *context, const WorkerResult *result)
{
    (void)context;
    if (jobs_publish_result(result) == 0)
        return 0;

    perror("jobs: publishing worker result");
    jobs_mark_failed(result->job_id);
    return -1;
}

static void enqueue_job(JobQueue *queue, const char *name)
{
    WorkerJob job = {0};
    size_t length = strlen(name);
    if (length == 0 || length >= sizeof(job.name) || length >= JOB_NAME_SIZE) {
        fprintf(stderr, "Usage: submit <job> (1-%zu bytes)\n",
                sizeof(job.name) - 1);
        return;
    }

    memcpy(job.name, name, length + 1);
    job.id = jobs_register(job.name);
    if (job.id < 0) {
        puts("Job list is full.");
        return;
    }

    int error = queue_push(queue, &job);
    if (error != 0) {
        jobs_mark_failed(job.id);
        fprintf(stderr, "submit: %s\n", strerror(error));
        return;
    }

    printf("Submitted Job %d: %s\n", job.id, job.name);
}

static void show_help(void)
{
    puts("MiniOS Available Commands");
    puts("  help                   - Show available commands");
    puts("  system                 - Show system uptime & memory");
    puts("  processes              - Show live Linux processes");
    puts("  run <program> [args]   - Run an external program");
    puts("  submit <job>           - Submit a new job");
    puts("  jobs                   - List submitted jobs");
    puts("  workers                - Show worker status");
    puts("  race                   - Compare shared counter updates");
    puts("  quit                   - Exit MiniOS");
}

static void run_command(char *arguments)
{
    size_t count = 0;
    const char *word = arguments;
    while (*word != '\0') {
        count++;
        word += strcspn(word, separators);
        word += strspn(word, separators);
    }

    if (count == 0) {
        puts("Usage: run <program> [args]");
        return;
    }

    char **args = calloc(count + 1, sizeof(*args));
    if (args == NULL) {
        perror("run: allocating arguments");
        return;
    }

    char *next;
    size_t index = 0;
    for (char *arg = strtok_r(arguments, separators, &next); arg != NULL;
         arg = strtok_r(NULL, separators, &next)) {
        args[index++] = arg;
    }
    args[index] = NULL;

    int status = run_program(args);
    if (status > 0)
        fprintf(stderr, "run: %s exited with status %d\n", args[0], status);
    free(args);
}

int shell_loop(void)
{
    JobQueue *queue = NULL;
    int error = queue_create(&queue, QUEUE_CAPACITY);
    if (error != 0) {
        fprintf(stderr, "queue: %s\n", strerror(error));
        return 1;
    }

    if (jobs_start() != 0) {
        queue_destroy(queue);
        return 1;
    }

    WorkerHooks hooks = {
        .context = queue,
        .take_job = queue_pop,
        .mark_running = mark_running,
        .publish_result = publish_result,
        .close_queue = queue_close
    };
    WorkerPool *pool = NULL;
    error = workers_start(&pool, WORKER_COUNT, &hooks);
    if (error != 0) {
        fprintf(stderr, "workers: %s\n", strerror(error));
        jobs_stop();
        queue_destroy(queue);
        return 1;
    }

    char *line = NULL;
    size_t capacity = 0;

    /* An external program can read input that follows its run command. */
    setvbuf(stdin, NULL, _IONBF, 0);

    puts("Starting MiniOS Shell...");
    puts("Type 'help' to see available commands.\n");

    for (;;) {
        printf("MiniOS> ");
        fflush(stdout);

        if (getline(&line, &capacity, stdin) < 0) {
            if (!feof(stdin))
                perror("MiniOS: reading input");
            break;
        }

        line[strcspn(line, "\r\n")] = '\0';
        char *remaining;
        char *command = strtok_r(line, separators, &remaining);
        if (command == NULL)
            continue;

        char *arguments = remaining + strspn(remaining, separators);

        if (strcmp(command, "help") == 0) {
            show_help();
        } else if (strcmp(command, "system") == 0) {
            show_system_info();
        } else if (strcmp(command, "processes") == 0) {
            show_processes();
        } else if (strcmp(command, "run") == 0) {
            run_command(arguments);
        } else if (strcmp(command, "submit") == 0) {
            enqueue_job(queue, arguments);
        } else if (strcmp(command, "jobs") == 0) {
            list_jobs();
        } else if (strcmp(command, "workers") == 0) {
            workers_print(pool, stdout);
        } else if (strcmp(command, "race") == 0) {
            error = race_run();
            if (error != 0)
                fprintf(stderr, "race: %s\n", strerror(error));
        } else if (strcmp(command, "quit") == 0) {
            puts("Exiting MiniOS. Goodbye!");
            break;
        } else {
            printf("Unknown command: '%s'. Type 'help' for available commands.\n",
                   command);
        }
    }

    free(line);
    workers_stop(pool);
    jobs_stop();
    queue_destroy(queue);
    return 0;
}
