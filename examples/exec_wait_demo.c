#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    pid_t pid;

    pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        printf("Child: executing date...\n");

        execlp("date", "date", NULL);

        perror("execlp");
        exit(1);
    }

    printf("Parent: waiting for child...\n");

    int status;

    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return 1;
    }

    printf("Parent: child finished.\n");

    return 0;
}
