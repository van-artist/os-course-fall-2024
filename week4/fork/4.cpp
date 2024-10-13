#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <sys/wait.h>
#define TIMES 1000
// 写入文本到文件的函数
ssize_t write_text(int file_id, const char *input_text)
{
    int count = 0;
    char ch = 0;
    do
    {
        ch = input_text[count];
        count++;
    } while (ch != 0);
    ssize_t bytes_written = write(file_id, input_text, count - 1);
    return bytes_written;
}

int main()
{

    int file_id = open("output.txt", O_RDWR | O_CREAT, 0644);
    if (file_id == -1)
    {
        std::cout << "父进程打开文件失败" << std::endl;
        return 1;
    }
    pid_t pid = fork();
    if (pid == 0)
    {
        lockf(file_id, F_LOCK, 0);
        for (int i = 0; i < TIMES; i++)
        {
            write_text(file_id, "Hello World! 2\n");
            usleep(1000);
        }
        lockf(file_id, F_ULOCK, 0);
        close(file_id);
        _exit(0);
    }
    else if (pid > 0)
    {
        // 父进程操作
        lockf(file_id, F_LOCK, 0);
        for (int i = 0; i < TIMES; i++)
        {
            write_text(file_id, "Hello World! 1\n");
            usleep(1000);
        }
        lockf(file_id, F_ULOCK, 0);

        close(file_id);
    }

    std::cout << std::endl;
    return 0;
}
