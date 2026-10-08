#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "system_info.h"

void show_help(void) {
    printf("\n========================================\n");
    printf("        MiniOS Available Commands        \n");
    printf("========================================\n");
    printf("  help      - Show available commands\n");
    printf("  system    - Show system uptime & memory\n");
    printf("  processes - Run process experiments\n");
    printf("  quit      - Exit MiniOS\n");
    printf("========================================\n\n");
}

void shell(void) {
    char command[50];

    printf("Starting MiniOS Shell...\n");
    printf("Type 'help' to see available commands.\n\n");

    while (1) {
        printf("MiniOS> ");
        if (scanf("%49s", command) != 1) {
            break;
        }

        if (strcmp(command, "help") == 0) {
            show_help();
        } 
        else if (strcmp(command, "system") == 0) {
            show_system_info();
        } 
        else if (strcmp(command, "processes") == 0) {
            printf("\nRunning Process Experiments...\n");
            printf("Check examples/ directory for fork, exec, and pipe demos.\n\n");
        } 
        else if (strcmp(command, "quit") == 0) {
            printf("Exiting MiniOS. Goodbye!\n");
            break;
        } 
        else {
            printf("Unknown command: '%s'. Type 'help' for available commands.\n", command);
        }
    }
}
