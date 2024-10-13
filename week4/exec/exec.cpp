#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t pid1 = fork();
    if (pid1 > 0)
    {
        printf("这是父进程!\n");
        pid_t pid2 = fork();
        if (pid2 == 0)
        {
            execv("./test", NULL);
        }
        else if (pid2 > 0)
        {
        }
        else if (pid2 < 0)
        {
            printf("进程创建失败");
        }
        wait(NULL);
    }
    else if (pid1 == 0)
    {
        execlp("ps", "ps", "-l", NULL);
    }
    else if (pid1 < 0)
    {
        printf("进程创建失败");
    }
    return 0;
}