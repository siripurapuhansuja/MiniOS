#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main(void)
{
    int fd[2];

    if (pipe(fd) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        /* Child reads from the pipe */
        close(fd[1]);

        char buffer[100];

        ssize_t n = read(fd[0], buffer, sizeof(buffer) - 1);

        if (n < 0) {
            perror("read");
            exit(1);
        }

        buffer[n] = '\0';

        printf("Child received: %s\n", buffer);

        close(fd[0]);

        exit(0);
    }

    /* Parent writes to the pipe */
    close(fd[0]);

    const char *message = "Hello from parent";

    if (write(fd[1], message, strlen(message)) < 0) {
        perror("write");
        return 1;
    }

    close(fd[1]);

    waitpid(pid, NULL, 0);

    return 0;
}
