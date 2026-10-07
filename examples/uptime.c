#include <stdio.h>
#include <string.h>

int main(void)
{
    FILE *fp;
    double uptime = 0.0;
    double idle = 0.0;
    char label[50];
    unsigned long memory = 0;

    fp = fopen("/proc/uptime", "r");

    if (fp == NULL)
    {
        perror("fopen");
        return 1;
    }

    if (fscanf(fp, "%lf %lf", &uptime, &idle) != 2)
    {
        fprintf(stderr, "Could not read /proc/uptime\n");
        fclose(fp);
        return 1;
    }

    fclose(fp);

    fp = fopen("/proc/meminfo", "r");

    if (fp == NULL)
    {
        perror("fopen");
        return 1;
    }

    while (fscanf(fp, "%49s %lu kB", label, &memory) == 2)
    {
        if (strcmp(label, "MemTotal:") == 0)
        {
            break;
        }
    }

    fclose(fp);

    printf("System uptime: %.2f seconds\n", uptime);
    printf("Idle time: %.2f seconds\n", idle);
    printf("Total memory: %lu kB\n", memory);

    return 0;
}
