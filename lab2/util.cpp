#include <iostream>
#include <sys/msg.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include "util.h"

int send_msg(int msgid, MessageFrame msg, int msgflg)
{
    size_t msg_size = sizeof(MessageFrame) - sizeof(long);

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
    size_t msg_size = sizeof(MessageFrame) - sizeof(long);

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

    while (true)
    {
        int result = msgrcv(msg_id, &msg_content, msg_size, 0, IPC_NOWAIT);
        if (result == -1)
        {
            if (errno == ENOMSG)
            {
                break;
            }
            else
            {
                throw MessageQueueException("Error clearing message queue: " + std::string(strerror(errno)));
            }
        }
    }

    int result = msgctl(msg_id, IPC_RMID, nullptr);
    if (result == -1)
    {
        throw MessageQueueException("Failed to delete message queue: " + std::string(strerror(errno)));
    }

    std::cout << "Message queue deleted successfully." << std::endl;
    return true;
}
