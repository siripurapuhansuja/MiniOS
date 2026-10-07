#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void pipe_example(void) {
    int pipefd[2];
    char write_msg[] = "Hello from Parent process through Pipe!";
    char read_msg[100];

    printf("\n--- Inter-Process Communication using Pipe ---\n");

    if (pipe(pipefd) == -1) {
        perror("Pipe creation failed");
        return;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return;
    }

    if (pid == 0) {
        // Child Process: Close unused write end
        close(pipefd[1]);
        
        // Read data sent from parent
        read(pipefd[0], read_msg, sizeof(read_msg));
        printf("Child Process (PID: %d) received message: \"%s\"\n", getpid(), read_msg);
        
        close(pipefd[0]);
        exit(0);
    } else {
        // Parent Process: Close unused read end
        close(pipefd[0]);
        
        printf("Parent Process (PID: %d) sending message to Child...\n", getpid());
        write(pipefd[1], write_msg, strlen(write_msg) + 1);
        
        close(pipefd[1]);
        
        // Sync with child process exit
        wait(NULL);
        printf("Parent Process: Communication completed successfully.\n");
    }
}

int main(void) {
    printf("========================================\n");
    printf("       Inter-Process Pipe Demo          \n");
    printf("========================================\n");

    pipe_example();

    return 0;
}
