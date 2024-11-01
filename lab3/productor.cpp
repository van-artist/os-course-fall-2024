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
    bool initialized;
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

ShmQueue init_shm_queue(int shm_key, int full_sem_key, int empty_sem_key, int mutex_sem_key)
{
    ShmQueue queue;

    queue.shm_id = shmget(shm_key, sizeof(ShmStruct), IPC_CREAT | 0666);
    if (queue.shm_id == -1)
    {
        throw std::runtime_error("共享内存分配失败");
    }

    ShmStruct *shm_addr = (ShmStruct *)shmat(queue.shm_id, nullptr, 0);
    if (shm_addr == (ShmStruct *)-1)
    {
        throw std::runtime_error("内存映射失败");
    }
    bool is_first_creation = !shm_addr->initialized;
    std::cout << "is_first_creation:" << is_first_creation << std::endl;
    if (is_first_creation)
    {
        shm_addr->out = 0;
        shm_addr->in = 0;
        shm_addr->initialized = true;
    }
    shmdt(shm_addr);

    queue.full_sem_id = semget(full_sem_key, 1, IPC_CREAT | 0666);
    queue.empty_sem_id = semget(empty_sem_key, 1, IPC_CREAT | 0666);
    queue.mutex_sem_id = semget(mutex_sem_key, 1, IPC_CREAT | 0666);

    semun arg;
    if (is_first_creation)
    {
        arg.val = 0;
        if (semctl(queue.full_sem_id, 0, SETVAL, arg) == -1)
            throw std::runtime_error("full信号量初始化失败");
        arg.val = BUFFER_QUEUE_LEN;
        if (semctl(queue.empty_sem_id, 0, SETVAL, arg) == -1)
            throw std::runtime_error("empty信号量初始化失败");
        arg.val = 1;
        if (semctl(queue.mutex_sem_id, 0, SETVAL, arg) == -1)
            throw std::runtime_error("mutex信号量初始化失败");
    }

    return queue;
}

Product produce_item()
{
    Product item;
    std::cout << "请输入要生产的内容: ";
    std::cin >> item;
    return item;
}

void insert_item(const ShmQueue &queue, const Product &item)
{
    ShmStruct *shm_addr = (ShmStruct *)shmat(queue.shm_id, nullptr, 0);
    if (shm_addr == (ShmStruct *)-1)
    {
        throw std::runtime_error("共享内存映射失败");
    }

    int index = shm_addr->in;
    strncpy((char *)shm_addr->buffer_queue[index], item.c_str(), BUFFER_SIZE - 1);
    shm_addr->buffer_queue[index][BUFFER_SIZE - 1] = '\0';
    shm_addr->in = (shm_addr->in + 1) % BUFFER_QUEUE_LEN;

    shmdt(shm_addr);
}

void delete_shm_queue(const ShmQueue &queue)
{
    if (shmctl(queue.shm_id, IPC_RMID, nullptr) == -1)
        throw std::runtime_error("删除共享内存失败");
    if (semctl(queue.empty_sem_id, 0, IPC_RMID) == -1)
        throw std::runtime_error("删除empty信号量失败");
    if (semctl(queue.full_sem_id, 0, IPC_RMID) == -1)
        throw std::runtime_error("删除full信号量失败");
    if (semctl(queue.mutex_sem_id, 0, IPC_RMID) == -1)
        throw std::runtime_error("删除mutex信号量失败");

    std::cout << "共享内存和信号量已成功删除" << std::endl;
}

void produce(const ShmQueue &queue)
{
    Product item = produce_item();
    P(queue.empty_sem_id);
    P(queue.mutex_sem_id);
    insert_item(queue, item);
    V(queue.mutex_sem_id);
    V(queue.full_sem_id);
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
        ShmQueue queue = init_shm_queue(SHM_KEY, FULL_SEM_KEY, EMPTY_SEM_KEY, MUTEX_SEM_KEY);
        bool running = true;

        std::cout << "获取共享内存区, ID: " << queue.shm_id << std::endl;
        while (running)
        {
            std::cout << "选择操作: 1. 生产产品  2. 退出  3. 删除共享内存和信号量" << std::endl;
            int choice;
            std::cin >> choice;
            if (std::cin.fail())
            {
                std::cin.clear();
                std::cin.ignore(1000, '\n');
                std::cerr << "无效选项" << std::endl;
                continue;
            }
            switch (choice)
            {
            case 1:
                produce(queue);
                scan_shm(queue);
                break;
            case 2:
                running = false;
                break;
            case 3:
                delete_shm_queue(queue);
                running = false;
                break;
            default:
                std::cerr << "无效选项" << std::endl;
                break;
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "错误: " << e.what() << std::endl;
    }
    return 0;
}
