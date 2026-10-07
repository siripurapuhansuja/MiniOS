#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    pid_t pid;
    int status;

    printf("Parent: starting exec/wait experiment\n");

    pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        printf("Child: executing 'ls -l'\n");

        execlp("ls", "ls", "-l", (char *)NULL);

        perror("execlp");
        return 1;
    }

    printf("Parent: waiting for child process...\n");

    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return 1;
    }

    if (WIFEXITED(status)) {
        printf("Parent: child exited with status %d\n",
               WEXITSTATUS(status));
    }

    printf("Parent: exec/wait experiment completed\n");

    return 0;
}
