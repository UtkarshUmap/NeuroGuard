#include "kernel/types.h"
#include "user/user.h"

int main(void)
{
    int pid;

    printf("Parent Process ID: %d\n", getpid());

    pid = fork();

    if(pid < 0)
    {
        printf("Fork failed!\n");
        exit(1);
    }
    else if(pid == 0)
    {
        printf("Child Process ID: %d\n", getpid());
        printf("Uptime: %d ticks\n", uptime());
        exit(0);
    }
    else
    {
        wait(0);
        printf("Child process has exited.\n");
        printf("Parent process completed.\n");
    }

    exit(0);
}
