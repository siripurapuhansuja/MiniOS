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
