#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include "sync.h"

typedef struct {
    JobQueue *queue;
    int expected_result;
} PushRequest;

static void push_job(JobQueue *queue, int id)
{
    WorkerJob job = {.id = id};
    assert(queue_push(queue, &job) == 0);
}

static void expect_job(JobQueue *queue, int id)
{
    WorkerJob job;
    assert(queue_pop(queue, &job) == 1);
    assert(job.id == id);
}

static void expect_closed(JobQueue *queue)
{
    WorkerJob job;
    assert(queue_pop(queue, &job) == 0);
}

static void *push_third(void *context)
{
    PushRequest *request = context;
    WorkerJob job = {.id = 3};
    assert(queue_push(request->queue, &job) == request->expected_result);
    return NULL;
}

static void *idle_worker(void *context)
{
    expect_closed(context);
    return NULL;
}

int main(void)
{
    JobQueue *queue;
    pthread_t thread;
    pthread_t waiters[4];
    const struct timespec pause = {.tv_nsec = 100000000};

    assert(queue_create(&queue, 2) == 0);
    push_job(queue, 1);
    push_job(queue, 2);
    queue_close(queue);
    expect_job(queue, 1);
    expect_job(queue, 2);
    expect_closed(queue);
    queue_destroy(queue);
    puts("PASS: FIFO order and drain after close");

    assert(queue_create(&queue, 2) == 0);
    push_job(queue, 1);
    push_job(queue, 2);
    PushRequest request = {.queue = queue, .expected_result = 0};
    assert(pthread_create(&thread, NULL, push_third, &request) == 0);
    expect_job(queue, 1);
    expect_job(queue, 2);
    expect_job(queue, 3);
    assert(pthread_join(thread, NULL) == 0);
    queue_close(queue);
    expect_closed(queue);
    queue_destroy(queue);
    puts("PASS: Three jobs through two slots");

    assert(queue_create(&queue, 2) == 0);
    for (int i = 0; i < 4; i++)
        assert(pthread_create(&waiters[i], NULL, idle_worker, queue) == 0);
    nanosleep(&pause, NULL);
    queue_close(queue);
    queue_close(queue);
    for (int i = 0; i < 4; i++)
        assert(pthread_join(waiters[i], NULL) == 0);
    queue_destroy(queue);
    puts("PASS: Close twice with four idle workers");

    assert(queue_create(&queue, 2) == 0);
    push_job(queue, 1);
    push_job(queue, 2);
    request = (PushRequest){.queue = queue, .expected_result = ECANCELED};
    for (int i = 0; i < 4; i++)
        assert(pthread_create(&waiters[i], NULL, push_third, &request) == 0);
    nanosleep(&pause, NULL);
    queue_close(queue);
    for (int i = 0; i < 4; i++)
        assert(pthread_join(waiters[i], NULL) == 0);
    expect_job(queue, 1);
    expect_job(queue, 2);
    expect_closed(queue);
    queue_destroy(queue);
    puts("PASS: Close a full queue with four producers");

    return 0;
}
