#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    int shared_variable = 100;

    printf("========================================\n");
    printf("     Fork Memory State Demonstration    \n");
    printf("========================================\n");
    printf("Initial Variable Value before fork: %d\n\n", shared_variable);

    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return 1;
    }

    if (pid == 0) {
        // Child modifies its copy of the variable
        shared_variable += 50;
        printf("Child Process (PID: %d):\n", getpid());
        printf("  Modified Variable Value = %d\n", shared_variable);
        printf("  Address of Variable = %p\n\n", (void *)&shared_variable);
        exit(0);
    } else {
        // Parent waits for child to finish
        wait(NULL);
        printf("Parent Process (PID: %d):\n", getpid());
        printf("  Unchanged Variable Value = %d\n", shared_variable);
        printf("  Address of Variable = %p\n\n", (void *)&shared_variable);
        printf("Conclusion: Parent and Child have separate memory spaces!\n");
    }

    return 0;
}
