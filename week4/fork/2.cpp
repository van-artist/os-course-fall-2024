#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    printf("hello world 1 \n");
    pid_t pid1 = fork();
    if (pid1 == 0)
    {
        printf("hello world 2 \n");
        _exit(0);
    }
    else if (pid1 > 0)
    {
        pid_t pid2 = fork();
        if (pid2 == 0)
        {
            printf("hello world 3 \n");
            _exit(0);
        }
        else if (pid2 > 0)
        {
        }
        else if (pid2 < 0)
        {
        }
    }
    else if (pid1 < 0)
    {
    }

    return 0;
}