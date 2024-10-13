#include <iostream>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t pid1 = fork();

    if (pid1 == 0)
    {
        std::cout << "b" << std::endl;

        _exit(0);
    }
    else if (pid1 > 0)
    {

        pid_t pid2 = fork();
        if (pid2 == 0)
        {

            std::cout << "c" << std::endl;

            _exit(0);
        }
        else if (pid2 > 0)
        {

            std::cout << "a" << std::endl;
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