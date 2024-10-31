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

// P操作
void P(int semid)
{
    struct sembuf p = {0, -1, 0};
    semop(semid, &p, 1);
}

// V操作
void V(int semid)
{
    struct sembuf v = {0, 1, 0};
    semop(semid, &v, 1);
}

int get_shm_queue(int shm_key)
{
    int shmid = shmget(shm_key, sizeof(ShmStruct), 0666);
    if (shmid == -1)
    {
        throw std::runtime_error("共享内存获取失败");
    }
    return shmid;
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

Product consum_item()
{
    Product product;
    ShmStruct *shm_addr = (ShmStruct *)shmat(shmid, nullptr, 0);
    if (shm_addr == (ShmStruct *)-1)
    {
        throw std::runtime_error("映射共享内存失败");
    }

    int index = shm_addr->out;
    product.assign((char *)shm_addr->buffer_queue[index]);
    shm_addr->out = (shm_addr->out + 1) % BUFFER_QUEUE_LEN;
    shmdt(shm_addr);
    return product;
}

void consum()
{
    P(full_semid);
    P(mutex_semid);

    Product product = consum_item();
    std::cout << "消费产品: " << product << std::endl;

    V(mutex_semid);
    V(empty_semid);
}

int main()
{
    try
    {
        shmid = get_shm_queue(SHM_KEY);
        full_semid = semget(FULL_SEM_KEY, 1, 0666);
        empty_semid = semget(EMPTY_SEM_KEY, 1, 0666);
        mutex_semid = semget(MUTEXT_SEM_KEY, 1, 0666);
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    while (true)
    {
        int op = -1;
        std::cout << "消费者进程: 1.消费产品；2.退出；3.删除信号量和共享内存" << std::endl;
        std::cin >> op;
        switch (op)
        {
        case 1:
            consum();
            break;
        case 2:
            return 0;
        case 3:
            delete_shm();
            return 0;
        default:
            break;
        }
    }
}
