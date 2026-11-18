#ifndef IPC_H
#define IPC_H

#include <stddef.h>
#include <sys/types.h>

ssize_t send_result(int fd, const char *result);
ssize_t receive_result(int fd, char *buf, size_t capacity);

#endif
