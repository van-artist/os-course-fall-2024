#include <unistd.h>
#include <sys/ipc.h>
#include <iostream>
#include <sys/shm.h>
#include <stdexcept>
#include <string>
#include <cstring>
#include <cstdint>
#include <sys/sem.h>

#define BUFFER_QUEUE_LEN 10
#define BUFFER_SIZE 1024
#define SHM_KEY 10
#define EMPTY_SEM_KEY 1
#define FULL_SEM_KEY 2
#define MUTEXT_SEM_KEY 3

struct ShmStruct
{
    int out;
    int in;
    uint8_t buffer_queue[BUFFER_QUEUE_LEN][BUFFER_SIZE];
};
union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};
typedef uint8_t *Buffer;
typedef Buffer *BufferQueue;
typedef std::string Product;

int shmid = -1;
int full_semid = -1;
int empty_semid = -1;
int mutex_semid = -1;

void P(int semid)
{
    struct sembuf p = {0, -1, 0};
    semop(semid, &p, 1);
}

void V(int semid)
{
    struct sembuf v = {0, 1, 0};
    semop(semid, &v, 1);
}
static int create_shm_queue(int shm_key)
{
    shmid = shmget(shm_key, sizeof(ShmStruct), IPC_CREAT | 0666);
    if (shmid == -1)
    {
        throw std::runtime_error("共享内存分配失败");
    }

    ShmStruct *shm_addr = (ShmStruct *)shmat(shmid, nullptr, 0);
    if (shm_addr == (ShmStruct *)-1)
    {
        throw std::runtime_error("内存映射失败");
    }

    shm_addr->out = 0;
    shm_addr->in = 0;
    return shmid;
}

void init_semaphore(int semid, int value)
{
    union semun arg;
    arg.val = value;
    if (semctl(semid, 0, SETVAL, arg) == -1)
    {
        throw std::runtime_error("信号量初始化失败");
    }
}

Product produce_item()
{
    Product str;
    std::cin >> str;
    return str;
}

void insert_item(Product item)
{
    ShmStruct *shm_addr = (ShmStruct *)shmat(shmid, nullptr, 0);
    if (shm_addr == (ShmStruct *)-1)
    {
        throw std::runtime_error("映射共享内存区失败");
    }
    int index = shm_addr->in;
    strncpy((char *)shm_addr->buffer_queue[index], item.c_str(), BUFFER_SIZE - 1);
    shm_addr->in = (shm_addr->in + 1) % BUFFER_QUEUE_LEN;
    shmdt(shm_addr);
}

void product()
{
    Product item = produce_item();
    P(empty_semid);
    P(mutex_semid);
    insert_item(item);
    V(mutex_semid);
    V(full_semid);
}

void delete_shm()
{

    if (semctl(empty_semid, 0, IPC_RMID) == -1)
    {
        throw std::runtime_error("删除空缓冲区信号量失败");
    }

    if (semctl(full_semid, 0, IPC_RMID) == -1)
    {
        throw std::runtime_error("删除已满缓冲区信号量失败");
    }

    if (semctl(mutex_semid, 0, IPC_RMID) == -1)
    {
        throw std::runtime_error("删除互斥锁信号量失败");
    }

    if (shmctl(shmid, IPC_RMID, nullptr) == -1)
    {
        throw std::runtime_error("删除共享内存区失败");
    }

    std::cout << "共享内存和信号量已成功删除" << std::endl;
}

void scan_shm()
{
    ShmStruct *shm_addr = (ShmStruct *)shmat(shmid, nullptr, 0);
    if (shm_addr == (ShmStruct *)-1)
    {
        throw std::runtime_error("内存映射失败");
    }
    std::cout << "共享内存内容:" << std::endl;
    for (int i = 0; i < BUFFER_QUEUE_LEN; i++)
    {
        for (int j = 0; j < BUFFER_SIZE; j++)
        {
            char ch = (char)(shm_addr->buffer_queue[i][j]);
            if (ch == 0)
            {
                break;
            }
            std::cout << ch;
        }
        std::cout << std::endl;
    }
}

int main()
{
    bool running = true;
    int shmid = -1;
    try
    {
        shmid = create_shm_queue(SHM_KEY);
        full_semid = semget(FULL_SEM_KEY, 1, IPC_CREAT | 0666);
        empty_semid = semget(EMPTY_SEM_KEY, 1, IPC_CREAT | 0666);
        mutex_semid = semget(MUTEXT_SEM_KEY, 1, IPC_CREAT | 0666);
        init_semaphore(full_semid, 0);
        init_semaphore(empty_semid, BUFFER_QUEUE_LEN);
        init_semaphore(mutex_semid, 1);
        std::cout << "创建/获取到共享内存:" << shmid << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
    while (running)
    {
        std::cout << "生产者进程: 1.生产产品；2.退出；3.删除信号量和共享内存" << std::endl;
        int op = -1;
        std::cin >> op;
        switch (op)
        {
        case 1:
            try
            {
                product();
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << std::endl;
            }
            scan_shm();
            break;
        case 2:
            running = false;
            break;
        case 3:
            try
            {
                delete_shm();
                running = false;
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << std::endl;
            }
            return 0;
            break;
        default:
            break;
        }
    }
    return 0;
}