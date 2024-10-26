#include <string>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <sys/msg.h>
#include "util.h"

int create_msgid()
{
    // 客户端只连接现有队列，不创建新队列
    int msgid = msgget(MSGKEY, 0666);
    if (msgid == -1)
    {
        throw std::runtime_error("Failed to connect to message queue");
    }
    return msgid;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Error: Missing required arguments." << std::endl;
        return -1;
    }

    int msg_type = std::stoi(argv[1]); // 获取消息类型
    int msg_id = -1;

    try
    {
        msg_id = create_msgid();
        std::cout << "Client " << getpid() << " : Message queue connected successfully, msgid: " << msg_id << std::endl;
    }
    catch (const std::runtime_error &e)
    {
        // 异常处理
        std::cerr << "Error connecting message queue in client: " << e.what() << std::endl;
        return -1;
    }

    // 循环发送消息
    for (int i = SEND_TIMES; i >= 1; i--)
    {
        MessageFrame msg;
        msg.msg_type = msg_type;
        msg.msg_content.pid = getpid();
        msg.msg_content.key = i;

        int result = send_msg(msg_id, msg, 0);
        if (result == -1)
        {
            std::cerr << "Client " << msg_type << " failed to send message." << std::endl;
            return -1;
        }

        // 接收服务器的响应
        MessageFrame response;
        // 设置为接收以自己PID为类型的消息
        response.msg_type = getpid();
        receive_msg(msg_id, &response, 0);

        std::cout << "Client " << getpid() << " received response from SERVER " << response.msg_content.pid << " with key "
                  << response.msg_content.key << std::endl;
    }

    return 0;
}
