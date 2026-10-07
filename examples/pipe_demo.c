#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main(void) {
    int fd[2];
    pid_t pid;
    char buffer[100];

    if (pipe(fd) == -1) {
        perror("pipe");
        return 1;
    }

    pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        close(fd[1]);

        read(fd[0], buffer, sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';

        printf("Child received: %s\n", buffer);

        close(fd[0]);
        return 0;
    }

    close(fd[0]);

    const char *message = "Hello from parent through pipe!";

    write(fd[1], message, strlen(message) + 1);

    close(fd[1]);

    wait(NULL);

    printf("Parent: pipe experiment completed\n");

    return 0;
}
