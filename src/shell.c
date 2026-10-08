#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

// Function for Member 4: Executes a single command
void execute_single_command(char **args) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        return;
    }
    if (pid == 0) {
        execvp(args[0], args);
        perror("Command execution failed");
        exit(1);
    } else {
        wait(NULL);
    }
}

// Function for Member 4: Executes two commands connected by a pipe (|)
void execute_piped_commands(char **cmd1, char **cmd2) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("Pipe creation failed");
        return;
    }

    pid_t pid1 = fork();
    if (pid1 == 0) {
        // Redirect stdout to pipe write end
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        execvp(cmd1[0], cmd1);
        perror("First command failed");
        exit(1);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        // Redirect stdin from pipe read end
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        execvp(cmd2[0], cmd2);
        perror("Second command failed");
        exit(1);
    }

    // Parent closes pipe ends and waits for both children
    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

int main(void) {
    printf("========================================\n");
    printf("     MiniOS Shell Execution Engine      \n");
    printf("========================================\n");

    // Mocking parsed data from Member 1
    // Command 1: "ls -l"
    char *cmd1[] = {"ls", "-l", NULL};
    // Command 2: "grep .c"
    char *cmd2[] = {"grep", ".c", NULL};

    printf("Executing pipeline simulation: ls -l | grep .c\n\n");
    execute_piped_commands(cmd1, cmd2);

    printf("\n[Execution Engine]: Completed cleanly.\n");
    return 0;
}
