#include <stdio.h>
#include <string.h>

int race_run(void);

int main(void)
{
    for (int trial = 1; trial <= 3; trial++) {
        printf("\nTrial %d\n", trial);
        int error = race_run();
        if (error != 0) {
            fprintf(stderr, "race: %s\n", strerror(error));
            return 1;
        }
    }
    return 0;
}
