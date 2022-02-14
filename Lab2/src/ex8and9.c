#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main()
{
    int pid;
    pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Fork failed!\n");
        exit(-1);
    }
    else if (pid==0) {
        printf("Child is printing: Child pid:%d, Parent pid:%d\n", getpid(), getppid());
        execlp("/bin/ps", "ps", NULL);
        printf("Still around...\n");
    }
    else {
        printf("Parent is printing: My Child pid:%d\n", pid);
        wait(NULL);
        exit(0);
    }
}
