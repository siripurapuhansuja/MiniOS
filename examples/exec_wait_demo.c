#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void execute_pipeline(void) {
    int pipefd[2];
    
    if (pipe(pipefd) == -1) {
        perror("Pipe creation failed");
        return;
    }

    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("First fork failed");
        return;
    }

    if (pid1 == 0) {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);

        char *args1[] = {"ls", "-l", NULL};
        execvp(args1[0], args1);
        
        perror("Exec on command 1 failed");
        exit(1);
    }

    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("Second fork failed");
        return;
    }

    if (pid2 == 0) {
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);

        char *args2[] = {"grep", ".c", NULL};
        execvp(args2[0], args2);
        
        perror("Exec on command 2 failed");
        exit(1);
    }

    close(pipefd[0]);
    close(pipefd[1]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    printf("\n[Shell Parent]: Pipeline executed cleanly.\n");
}

int main(void) {
    printf("========================================\n");
    printf("     Member 4: Pipeline Testing         \n");
    printf("========================================\n");
    execute_pipeline();
    return 0;
}
