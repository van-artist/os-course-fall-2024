#include <unistd.h>
#include <sys/ipc.h>
#include <iostream>
#include <sys/shm.h>
#include <stdexcept>
#include <string>
#include <cstring>
#include <algorithm>

typedef char unit_8;

const int N = 10;
const int BUFFER_SIZE = 1024;
const int SHM_KEY = 10;

struct Buffer
{
    unit_8 data[BUFFER_SIZE];
};

struct ShmStruct
{
    int in;
    int out;
    Buffer bufferQueue[N];
};
std::string comsum(int shmid, int index)
{
    std::string output;
    char *shm_addr = (char *)shmat(shmid, nullptr, 0);

    return output;
}
int main()
{
    while (true)
    {
        int op = -1;
        std::string prompt("消费者进程: 1.消费产品；2.退出；3.删除信号量和共享内存");
        std::cin >> op;
        std::string products;
        switch (op)
        {
        case 1:
            break;
        case 2:
            break;
        case 3:
            break;
        default:
            break;
        }
    }
    return 0;
}