#define _POSIX_C_SOURCE 200809L
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "minios.h"

void show_processes(void)
{
    DIR *proc = opendir("/proc");
    if (proc == NULL) {
        perror("processes: opening /proc");
        return;
    }

    printf("%-8s %s\n", "PID", "NAME");
    for (;;) {
        errno = 0;
        struct dirent *entry = readdir(proc);
        if (entry == NULL) {
            if (errno != 0)
                perror("processes: reading /proc");
            break;
        }

        if (entry->d_name[strspn(entry->d_name, "0123456789")] != '\0')
            continue;

        char path[sizeof(entry->d_name) + sizeof("/proc//comm")];
        snprintf(path, sizeof(path), "/proc/%s/comm", entry->d_name);
        FILE *file = fopen(path, "r");
        if (file == NULL)
            continue;

        char name[256];
        if (fgets(name, sizeof(name), file) != NULL) {
            name[strcspn(name, "\n")] = '\0';
            printf("%-8s %s\n", entry->d_name, name);
        }
        fclose(file);
    }

    closedir(proc);
}

int run_program(char **args)
{
    pid_t pid = fork();
    if (pid < 0) {
        perror("run: fork");
        return -1;
    }

    if (pid == 0) {
        execvp(args[0], args);
        int error = errno;
        perror(args[0]);
        _exit(error == ENOENT ? 127 : 126);
    }

    int status;
    pid_t waited;
    do {
        waited = waitpid(pid, &status, 0);
    } while (waited < 0 && errno == EINTR);

    if (waited < 0) {
        perror("run: waitpid");
        return -1;
    }

    if (WIFSIGNALED(status)) {
        fprintf(stderr, "run: %s terminated by signal %d\n", args[0], WTERMSIG(status));
        return -1;
    }

    return WEXITSTATUS(status);
}
