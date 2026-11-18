#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "ipc.h"

ssize_t send_result(int fd, const char *result) {
    if (result == NULL) {
        errno = EINVAL;
        return -1;
    }

    size_t length = strlen(result);
    size_t sent = 0;

    while (sent < length) {
        ssize_t n = write(fd, result + sent, length - sent);

        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }

        if (n == 0) {
            errno = EIO;
            return -1;
        }

        sent += (size_t)n;
    }

    return (ssize_t)sent;
}

ssize_t receive_result(int fd, char *buf, size_t capacity) {
    if (buf == NULL || capacity == 0) {
        errno = EINVAL;
        return -1;
    }

    ssize_t n;

    do {
        n = read(fd, buf, capacity - 1);
    } while (n < 0 && errno == EINTR);

    if (n < 0)
        return -1;

    buf[n] = '\0';
    return n;
}

/*
 * Send one complete WorkerResult through a pipe.
 * The message is small enough for an atomic pipe write.
 */
int send_worker_result(int fd, const WorkerResult *result) {
    if (result == NULL) {
        errno = EINVAL;
        return -1;
    }

    ssize_t n;

    do {
        n = write(fd, result, sizeof(*result));
    } while (n < 0 && errno == EINTR);

    if (n != (ssize_t)sizeof(*result)) {
        if (n >= 0)
            errno = EIO;
        return -1;
    }

    return 0;
}

/*
 * Read exactly one WorkerResult.
 *
 * Returns:
 *   1  = complete result received
 *   0  = clean EOF (pipe closed)
 *  -1  = error or incomplete result
 */
int receive_worker_result(int fd, WorkerResult *result) {
    if (result == NULL) {
        errno = EINVAL;
        return -1;
    }

    unsigned char *buf = (unsigned char *)result;
    size_t received = 0;

    while (received < sizeof(*result)) {
        ssize_t n = read(fd,
                         buf + received,
                         sizeof(*result) - received);

        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }

        if (n == 0) {
            if (received == 0)
                return 0;

            errno = EIO;
            return -1;
        }

        received += (size_t)n;
    }

    return 1;
}
