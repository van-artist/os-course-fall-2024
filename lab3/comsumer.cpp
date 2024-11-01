#include <iostream>
#include <stdexcept>
#include <string>
#include <cstring>
#include <cstdint>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>

constexpr int BUFFER_QUEUE_LEN = 10;
constexpr int BUFFER_SIZE = 1024;
constexpr int SHM_KEY = 10;
constexpr int EMPTY_SEM_KEY = 1;
constexpr int FULL_SEM_KEY = 2;
constexpr int MUTEX_SEM_KEY = 3;

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

struct ShmQueue
{
    int shm_id;
    int full_sem_id;
    int empty_sem_id;
    int mutex_sem_id;
};

typedef std::string Product;

void semaphore_op(int sem_id, int op)
{
    struct sembuf operation = {0, static_cast<short>(op), 0};
    if (semop(sem_id, &operation, 1) == -1)
    {
        throw std::runtime_error("信号量操作失败");
    }
}

void P(int sem_id) { semaphore_op(sem_id, -1); }
void V(int sem_id) { semaphore_op(sem_id, 1); }

ShmQueue get_shm_queue(int shm_key, int full_sem_key, int empty_sem_key, int mutex_sem_key)
{
    ShmQueue queue;

    queue.shm_id = shmget(shm_key, sizeof(ShmStruct), 0666);
    if (queue.shm_id == -1)
    {
        throw std::runtime_error("共享内存获取失败");
    }

    queue.full_sem_id = semget(full_sem_key, 1, 0666);
    queue.empty_sem_id = semget(empty_sem_key, 1, 0666);
    queue.mutex_sem_id = semget(mutex_sem_key, 1, 0666);

    if (queue.full_sem_id == -1 || queue.empty_sem_id == -1 || queue.mutex_sem_id == -1)
    {
        throw std::runtime_error("信号量获取失败");
    }

    return queue;
}

void delete_shm_queue(const ShmQueue &queue)
{
    if (semctl(queue.empty_sem_id, 0, IPC_RMID) == -1)
        throw std::runtime_error("删除empty信号量失败");
    if (semctl(queue.full_sem_id, 0, IPC_RMID) == -1)
        throw std::runtime_error("删除full信号量失败");
    if (semctl(queue.mutex_sem_id, 0, IPC_RMID) == -1)
        throw std::runtime_error("删除mutex信号量失败");
    if (shmctl(queue.shm_id, IPC_RMID, nullptr) == -1)
        throw std::runtime_error("删除共享内存失败");

    std::cout << "共享内存和信号量已成功删除" << std::endl;
}

Product consume_item(const ShmQueue &queue)
{
    ShmStruct *shm_addr = (ShmStruct *)shmat(queue.shm_id, nullptr, 0);
    if (shm_addr == (ShmStruct *)-1)
    {
        throw std::runtime_error("映射共享内存失败");
    }

    int index = shm_addr->out;
    Product product((char *)shm_addr->buffer_queue[index]);
    shm_addr->out = (shm_addr->out + 1) % BUFFER_QUEUE_LEN;

    shmdt(shm_addr);
    return product;
}

void consume(const ShmQueue &queue)
{
    P(queue.full_sem_id);
    P(queue.mutex_sem_id);

    Product product = consume_item(queue);
    std::cout << "消费产品: " << product << std::endl;

    V(queue.mutex_sem_id);
    V(queue.empty_sem_id);
}

void scan_shm(const ShmQueue &queue)
{
    ShmStruct *shm_addr = (ShmStruct *)shmat(queue.shm_id, nullptr, 0);
    if (shm_addr == (ShmStruct *)-1)
    {
        throw std::runtime_error("共享内存映射失败");
    }

    std::cout << "共享内存内容:" << std::endl;
    for (int i = shm_addr->out; i != shm_addr->in; i = (i + 1) % BUFFER_QUEUE_LEN)
    {
        std::cout << "索引 " << i << " -> 内容: ";
        for (int j = 0; j < BUFFER_SIZE; j++)
        {
            char ch = (char)(shm_addr->buffer_queue[i][j]);
            if (ch == '\0')
            {
                break;
            }
            std::cout << ch;
        }
        std::cout << std::endl;
    }
    shmdt(shm_addr);
}

int main()
{
    try
    {
        ShmQueue queue = get_shm_queue(SHM_KEY, FULL_SEM_KEY, EMPTY_SEM_KEY, MUTEX_SEM_KEY);
        std::cout << "获取共享内存区,ID: " << queue.shm_id << std::endl;
        bool running = true;

        while (running)
        {
            std::cout << "消费者进程: 1.消费产品；2.查看共享内存内容；3.删除信号量和共享内存；4.退出" << std::endl;
            int op;
            std::cin >> op;

            if (std::cin.fail())
            {
                std::cin.clear();
                std::cin.ignore(1000, '\n');
                std::cerr << "无效选项" << std::endl;
                continue;
            }

            switch (op)
            {
            case 1:
                consume(queue);
                break;
            case 2:
                try
                {
                    delete_shm_queue(queue);
                    running = false;
                }
                catch (const std::exception &e)
                {
                    std::cerr << "删除共享内存和信号量时出错: " << e.what() << std::endl;
                }
                break;
            case 3:
                running = false;
                break;
            default:
                std::cerr << "无效选项" << std::endl;
                break;
            }
        }
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
