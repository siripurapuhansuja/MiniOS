#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

// 1. Basic Fork Example
void basic_fork(void) {
    printf("\n--- 1. Basic Fork ---\n");
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        return;
    }
    if (pid == 0) {
        printf("Child Process executed! PID: %d, Parent PID: %d\n", getpid(), getppid());
        exit(0);
    } else {
        wait(NULL);
        printf("Parent Process executed! PID: %d, Child PID: %d\n", getpid(), pid);
    }
}

// 2. Parent and Child Execution Order
void parent_child(void) {
    printf("\n--- 2. Parent and Child Execution ---\n");
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        return;
    }
    if (pid == 0) {
        printf("Child is working...\n");
        sleep(1);
        printf("Child finished.\n");
        exit(0);
    } else {
        printf("Parent is waiting for child...\n");
        wait(NULL);
        printf("Parent continues after child finished.\n");
    }
}

// 3. Zombie Process Demonstration
void zombie_example(void) {
    printf("\n--- 3. Zombie Process Example ---\n");
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        return;
    }
    if (pid == 0) {
        printf("Child (PID: %d) terminating immediately...\n", getpid());
        exit(0);
    } else {
        printf("Parent (PID: %d) sleeping for 3 seconds without calling wait()...\n", getpid());
        printf("Check 'ps aux | grep defunc' in another terminal to see Zombie state.\n");
        sleep(3);
        wait(NULL);
        printf("Parent cleaned up child zombie process.\n");
    }
}

// 4. Orphan Process Demonstration
void orphan_example(void) {
    printf("\n--- 4. Orphan Process Example ---\n");
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        return;
    }
    if (pid == 0) {
        printf("Child (PID: %d) Original Parent PID: %d\n", getpid(), getppid());
        printf("Child sleeping to allow parent to exit first...\n");
        sleep(2);
        printf("Child (PID: %d) New Adopted Parent PID (init/systemd): %d\n", getpid(), getppid());
        exit(0);
    } else {
        printf("Parent (PID: %d) terminating immediately...\n", getpid());
        exit(0);
    }
}

int main(void) {
    printf("========================================\n");
    printf("       Process Management Demos         \n");
    printf("========================================\n");

    basic_fork();
    parent_child();
    zombie_example();
    
    printf("\nRunning Orphan Example (Parent will exit now)...\n");
    orphan_example();

    return 0;
}
