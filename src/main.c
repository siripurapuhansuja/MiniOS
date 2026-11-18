#include <stdio.h>
#include "minios.h"
#include "runtime.h"

int main(void)
{
    printf("\n========================================\n");
    printf("         Welcome to MiniOS v1.0         \n");
    printf("========================================\n\n");

    if (runtime_start() != 0) {
        fprintf(stderr, "MiniOS initialization failed.\n");
        return 1;
    }

    int status = shell_loop();

    runtime_stop();

    return status;
}
