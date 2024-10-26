#include <stdio.h>
#include <unistd.h> // 用于usleep函数

void clear_screen()
{
    printf("\033[H\033[J"); // ANSI转义序列，用于清屏
}

int main()
{
    int position = 0;
    printf("Hello Wordl!\n");
    return 0;
}
