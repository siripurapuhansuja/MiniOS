#include <stdio.h>
#include "minios.h"

int main(void)
{
    printf("\n========================================\n");
    printf("         Welcome to MiniOS v1.0         \n");
    printf("========================================\n\n");

    return shell_loop();
}
