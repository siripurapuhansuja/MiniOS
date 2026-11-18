#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jobs.h"
#include "minios.h"
#include "system_info.h"
#include "runtime.h"

static const char separators[] = " \t\r\n\v\f";

static void show_help(void)
{
    puts("MiniOS Available Commands");
    puts("  help                   - Show available commands");
    puts("  system                 - Show system uptime & memory");
    puts("  processes              - Show live Linux processes");
    puts("  run <program> [args]   - Run an external program");
    puts("  submit <job>           - Submit a new job");
    puts("  jobs                   - List submitted jobs");
    puts("  quit                   - Exit MiniOS");
}

static void run_command(char *arguments)
{
    size_t count = 0;
    const char *word = arguments;
    while (*word != '\0') {
        count++;
        word += strcspn(word, separators);
        word += strspn(word, separators);
    }

    if (count == 0) {
        puts("Usage: run <program> [args]");
        return;
    }

    char **args = calloc(count + 1, sizeof(*args));
    if (args == NULL) {
        perror("run: allocating arguments");
        return;
    }

    char *next;
    size_t index = 0;
    for (char *arg = strtok_r(arguments, separators, &next); arg != NULL;
         arg = strtok_r(NULL, separators, &next)) {
        args[index++] = arg;
    }
    args[index] = NULL;

    int status = run_program(args);
    if (status > 0)
        fprintf(stderr, "run: %s exited with status %d\n", args[0], status);
    free(args);
}

void shell_loop(void)
{
    char *line = NULL;
    size_t capacity = 0;

    /* An external program can read input that follows its run command. */
    setvbuf(stdin, NULL, _IONBF, 0);

    puts("Starting MiniOS Shell...");
    puts("Type 'help' to see available commands.\n");

    for (;;) {
        printf("MiniOS> ");
        fflush(stdout);

        if (getline(&line, &capacity, stdin) < 0) {
            if (!feof(stdin))
                perror("MiniOS: reading input");
            break;
        }

        line[strcspn(line, "\r\n")] = '\0';
        char *remaining;
        char *command = strtok_r(line, separators, &remaining);
        if (command == NULL)
            continue;

        char *arguments = remaining + strspn(remaining, separators);

        if (strcmp(command, "help") == 0) {
            show_help();
        } else if (strcmp(command, "system") == 0) {
            show_system_info();
        } else if (strcmp(command, "processes") == 0) {
            show_processes();
        } else if (strcmp(command, "run") == 0) {
            run_command(arguments);
        } else if (strcmp(command, "submit") == 0) {
            runtime_submit(arguments);
        } else if (strcmp(command, "jobs") == 0) {
            list_jobs();
        } else if (strcmp(command, "quit") == 0) {
            puts("Exiting MiniOS. Goodbye!");
            break;
        } else {
            printf("Unknown command: '%s'. Type 'help' for available commands.\n",
                   command);
        }
    }

    free(line);
}
