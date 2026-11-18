
#ifndef IPC_H
#define IPC_H

#include <stddef.h>
#include <sys/types.h>
#include "workers.h"

/* Existing text-based IPC */
ssize_t send_result(int fd, const char *result);
ssize_t receive_result(int fd, char *buf, size_t capacity);

/* Binary IPC for worker results */
int send_worker_result(int fd, const WorkerResult *result);
int receive_worker_result(int fd, WorkerResult *result);

#endif
