#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void shell(void) {
    char command[100];

    while (1) {
        printf("MiniOS> ");
        fflush(stdout);

        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }

        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "processes") == 0) {
            printf("\nProcess Experiments:\n");
            printf("1. exec + fork + wait\n");
            printf("2. pipe IPC\n");
            printf("\nRun the experiments using:\n");
            printf("./exec_wait_demo\n");
            printf("./pipe_demo\n\n");
        }
        else if (strcmp(command, "exit") == 0) {
            printf("Exiting MiniOS...\n");
            break;
        }
        else if (strlen(command) == 0) {
            continue;
        }
        else {
            printf("Unknown command: %s\n", command);
            printf("Available commands: processes, exit\n");
        }
    }
}
