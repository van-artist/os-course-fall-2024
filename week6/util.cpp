    #include <iostream>
    #include <sys/msg.h>
    #include <unistd.h>
    #include <sys/ipc.h>
    #include <cerrno>
    #include <cstring>
    #include <stdexcept>
    #include <string>
    #include <sstream>
    #include <vector>
    #include "util.h"

    char *string_to_text_ptr(std::string str)
    {
        char *msg_text = (char *)malloc((str.size() + 1) * sizeof(char));
        if (msg_text)
        {
            strcpy(msg_text, str.c_str());
        }
        else
        {
            std::cerr << "Memory allocation failed" << std::endl;
        }
        return msg_text;
    }

    char digit_to_char(int digit)
    {
        if (digit >= 0 && digit <= 9)
        {
            return '0' + digit;
        }
        return '?';
    }

    int char_to_digit(char ch)
    {
        int result = ch - '0';
        if (result >= 0 && result <= 9)
        {
            return result;
        }
        return -1;
    }

    int send_msg(int msgid, MessageFrame msg, int msgflg)
    {
        // 计算 MessageFrame 结构体的大小，不包括 msg_type
        size_t msg_size = sizeof(MessageFrame) - sizeof(long);

        // 发送消息，msg_size 是结构体大小
        int result = msgsnd(msgid, &msg, msg_size, msgflg);
        if (result == -1)
        {
            std::cerr << "Failed to send message: " << strerror(errno) << std::endl;
            return -1;
        }
        return result;
    }

    ssize_t receive_msg(int msg_id, MessageFrame *msgp, int msgflg)
    {
        // 计算 MessageFrame 的大小，不包括 msg_type
        size_t msg_size = sizeof(MessageFrame) - sizeof(long);

        // 接收消息，msg_size 是要接收的结构体大小
        ssize_t result = msgrcv(msg_id, (void *)msgp, msg_size, msgp->msg_type, msgflg);
        if (result == -1)
        {
            std::cerr << "Failed to receive message: " << strerror(errno) << std::endl;
            return -1;
        }

        return result;
    }

    bool shut_down_msg(int msg_id)
    {
        MessageFrame msg_content;
        size_t msg_size = sizeof(MessageFrame) - sizeof(long);

        // 清空所有消息
        while (true)
        {
            int result = msgrcv(msg_id, &msg_content, msg_size, 0, IPC_NOWAIT);
            if (result == -1)
            {
                // 如果消息队列为空，退出循环
                if (errno == ENOMSG)
                {
                    break; // 队列已经清空，正常退出
                }
                else
                {
                    // 如果不是消息队列为空的错误，抛出异常
                    throw MessageQueueException("Error clearing message queue: " + std::string(strerror(errno)));
                }
            }

            // 打印清空的消息内容 (可选，根据需求)
            std::cout << "Cleared message from PID: " << msg_content.msg_content.pid
                    << " with key: " << msg_content.msg_content.key << std::endl;
        }

        // 删除消息队列
        int result = msgctl(msg_id, IPC_RMID, nullptr);
        if (result == -1)
        {
            throw MessageQueueException("Failed to delete message queue: " + std::string(strerror(errno)));
        }

        std::cout << "Message queue deleted successfully." << std::endl;
        return true;
    }

    std::vector<std::string> split(const std::string &str, char delimiter)
    {
        std::vector<std::string> tokens;
        std::stringstream ss(str);
        std::string token;

        while (std::getline(ss, token, delimiter))
        {
            tokens.push_back(token);
        }

        return tokens;
    }
