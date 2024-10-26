#include <unistd.h>
#include <sys/ipc.h>
#include <iostream>
#include <sys/shm.h>
#include <stdexcept>
#include <string>
#include <cstring>

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

int create_shm_queue(int shm_key)
{
    int shmid = -1;
    shmid = shmget(shm_key, sizeof(ShmStruct), IPC_CREAT | 0666);
    return shmid;
}
void product(int shmid, int index, std::string products)
{
    char *shm_addr = (char *)shmat(shmid, nullptr, 0);
    if (shm_addr == (char *)-1)
    {
        throw std::runtime_error("映射共享内存区失败");
    }
    strncpy(shm_addr, products.c_str(), BUFFER_SIZE - 1); // 这里是为了限制内存操作大小(1024B),避免溢出
}

void delete_shm(int shmid)
{
    int result = shmctl(shmid, IPC_RMID, nullptr);
    if (result == -1)
    {
        throw std::runtime_error("删除共享内存区失败");
    }
}
int main()
{
    int shmid = -1;
    try
    {
        shmid = create_shm_queue(SHM_KEY);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
    while (true)
    {
        int op = -1;
        std::string prompt("生产者进程: 1.生产产品；2.退出；3.删除信号量和共享内存");
        std::cin >> op;
        std::string products;
        switch (op)
        {
        case 1:
            std::cin >> products;
            try
            {
                product(shmid, 0, products);
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << std::endl;
            }

            break;
        case 2:
            return;
            break;
        case 3:
            try
            {
                delete_shm(shmid);
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << std::endl;
            }
            break;
        default:
            break;
        }
    }
    return 0;
}