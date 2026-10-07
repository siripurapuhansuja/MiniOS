#include <stdio.h>

int main(void)
{
    FILE *fp = fopen("/proc/uptime", "r");

    if (fp == NULL) {
        perror("fopen");
        return 1;
    }

    double uptime;

    if (fscanf(fp, "%lf", &uptime) != 1) {
        fprintf(stderr, "Could not read /proc/uptime\n");
        fclose(fp);
        return 1;
    }

    printf("System uptime: %.2f seconds\n", uptime);

    fclose(fp);

FILE *mem_fp = fopen("/proc/meminfo", "r");

if (mem_fp == NULL) {
    perror("fopen");
    return 1;
}

char label[50];
long mem_total;

if (fscanf(mem_fp, "%49s %ld", label, &mem_total) != 2) {
    fprintf(stderr, "Could not read /proc/meminfo\n");
    fclose(mem_fp);
    return 1;
}

printf("Total memory: %ld kB\n", mem_total);

fclose(mem_fp);
return 0;
}
