#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>

enum {
    RACE_THREADS = 4,
    RACE_INCREMENTS = 100000
};

typedef struct {
    atomic_int counter;
    pthread_mutex_t lock;
} RaceDemo;

static void *increment_unprotected(void *context)
{
    RaceDemo *demo = context;

    for (int i = 0; i < RACE_INCREMENTS; i++) {
        int value = atomic_load_explicit(&demo->counter, memory_order_relaxed);
        atomic_store_explicit(&demo->counter, value + 1, memory_order_relaxed);
    }

    return NULL;
}

static void *increment_protected(void *context)
{
    RaceDemo *demo = context;

    for (int i = 0; i < RACE_INCREMENTS; i++) {
        pthread_mutex_lock(&demo->lock);
        int value = atomic_load_explicit(&demo->counter, memory_order_relaxed);
        atomic_store_explicit(&demo->counter, value + 1, memory_order_relaxed);
        pthread_mutex_unlock(&demo->lock);
    }

    return NULL;
}

static int run_trial(RaceDemo *demo, void *(*increment)(void *), int *total)
{
    pthread_t threads[RACE_THREADS];
    int started = 0;
    int error = 0;

    atomic_store_explicit(&demo->counter, 0, memory_order_relaxed);

    for (; started < RACE_THREADS; started++) {
        error = pthread_create(&threads[started], NULL, increment, demo);
        if (error != 0)
            break;
    }

    for (int i = 0; i < started; i++)
        pthread_join(threads[i], NULL);

    if (error == 0)
        *total = atomic_load_explicit(&demo->counter, memory_order_relaxed);

    return error;
}

int race_run(void)
{
    RaceDemo demo;
    int unprotected_total;
    int protected_total;

    atomic_init(&demo.counter, 0);

    int error = pthread_mutex_init(&demo.lock, NULL);
    if (error != 0)
        return error;

    error = run_trial(&demo, increment_unprotected, &unprotected_total);
    if (error == 0)
        error = run_trial(&demo, increment_protected, &protected_total);

    pthread_mutex_destroy(&demo.lock);
    if (error != 0)
        return error;

    printf("Expected total: %d\n", RACE_THREADS * RACE_INCREMENTS);
    printf("Without mutex:  %d\n", unprotected_total);
    printf("With mutex:     %d\n", protected_total);
    return 0;
}