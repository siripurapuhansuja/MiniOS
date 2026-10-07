#include <stdio.h>
#include <string.h>
#include "system_info.h"

void show_system_info(void)
{
    FILE *fp;
    double uptime = 0.0;
    double idle = 0.0;
    char label[50];
    unsigned long memory = 0;

    fp = fopen("/proc/uptime", "r");

    if (fp == NULL)
    {
        perror("Unable to open /proc/uptime");
        return;
    }

    if (fscanf(fp, "%lf %lf", &uptime, &idle) != 2)
    {
        fprintf(stderr, "Error reading /proc/uptime\n");
        fclose(fp);
        return;
    }

    fclose(fp);

    fp = fopen("/proc/meminfo", "r");

    if (fp == NULL)
    {
        perror("Unable to open /proc/meminfo");
        return;
    }

    while (fscanf(fp, "%49s %lu kB", label, &memory) == 2)
    {
        if (strcmp(label, "MemTotal:") == 0)
        {
            break;
        }
    }

    fclose(fp);

    printf("\n========================================\n");
    printf("           System Information\n");
    printf("========================================\n");
    printf("System Uptime : %.2f seconds\n", uptime);
    printf("Idle Time     : %.2f seconds\n", idle);
    printf("Total Memory  : %lu kB\n", memory);
    printf("========================================\n\n");
}
