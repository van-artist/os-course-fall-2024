#include <iostream>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <vector>
#include <stdexcept>
#include "util.h"

int create_msgid()
{
    int msgid = msgget(MSGKEY, 0666 | IPC_CREAT);
    if (msgid == -1)
    {
        throw std::runtime_error("Failed to create message queue");
    }
    return msgid;
}

int main()
{
    std::cout << "Server started" << std::endl;
    int msg_id = create_msgid();
    std::vector<int> client_types = {CLIENT_TYPE_1, CLIENT_TYPE_2};

    // 启动子进程
    for (const int &client_type : client_types)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            const char *args[] = {"./out/client", std::to_string(client_type).c_str(), NULL};
            execv("./out/client", (char *const *)args);
            _exit(0); // Use _exit(0) to terminate the child process immediately
        }
    }

    // 父进程
    while (true)
    {
        try
        {
            for(const int &client_type : client_types){
MessageFrame msg_content;
            msg_content.msg_type = 0;

            // 接收消息
            auto result = receive_msg(msg_id, &msg_content, 0);
            if (result == -1)
            {
                std::cerr << "Error receiving message." << std::endl;
                continue;
            }

            // 处理接收到的消息
            int client_pid = msg_content.msg_content.pid;
            int key = msg_content.msg_content.key;

            std::string res_str = "SERVER " + std::to_string(getpid()) +
                                  " received message " + std::to_string(key) +
                                  " from CLIENT " + std::to_string(client_pid);
            std::cout << res_str << std::endl;

            // 服务器发送响应
            MessageFrame response;
            // 使用客户端的PID作为消息类型，确保消息能被正确的客户端接收
            response.msg_type = client_pid;
            response.msg_content.pid = getpid();
            response.msg_content.key = key;

            send_msg(msg_id, response, 0);

            // 如果收到的 key 是 1，则关闭消息队列
            if (key == 1)
            {
                shut_down_msg(msg_id);
                std::cout << "Message queue deleted successfully" << std::endl;
                return 0;
            }
            }
            
        }
        catch (const MessageQueueException &e)
        {
            std::cerr << "Exception caught: " << e.what() << std::endl;
            break;
        }
    }

    return 0;
}
