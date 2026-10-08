#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void run_process_experiment(const char *program)
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        execl(program, program, (char *)NULL);

        perror("exec");
        exit(1);
    }

    waitpid(pid, NULL, 0);
}

void shell(void)
{
    char command[100];

    printf("MiniOS\n");

    while (1) {
        printf("MiniOS> ");

        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }

        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "processes") == 0) {

            printf("\nProcess Experiments\n");
            printf("-------------------\n");

            printf("MiniOS PID: %d\n", getpid());
            printf("Parent PID: %d\n", getppid());

            printf("\nRunning fork + exec + wait experiment...\n");
            run_process_experiment("./exec_wait_demo");

            printf("\nRunning pipe IPC experiment...\n");
            run_process_experiment("./pipe_demo");

            printf("\nProcess experiments completed.\n\n");
        }
        else if (strcmp(command, "help") == 0) {

            printf("\nMiniOS Commands:\n");
            printf("  processes  - Run process and IPC experiments\n");
            printf("  help       - Show available commands\n");
            printf("  quit       - Exit MiniOS\n\n");
        }
        else if (strcmp(command, "quit") == 0) {

            printf("Exiting MiniOS...\n");
            break;
        }
        else if (strlen(command) == 0) {

            continue;
        }
        else {

            printf("Unknown command: %s\n", command);
            printf("Type 'help' to see available commands.\n");
        }
    }
}
