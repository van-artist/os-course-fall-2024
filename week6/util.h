#ifndef UTIL_H
#define UTIL_H
#include <string>
#include <vector>
#include <unistd.h>

#define MSGKEY 75
#define CLIENT_TYPE_1 1
#define CLIENT_TYPE_2 2
#define MAX_TEXT_SIZE 1024
#define SEND_TIMES 10

class MessageQueueException : public std::runtime_error
{
public:
    explicit MessageQueueException(const std::string &msg) : std::runtime_error(msg) {}
};

struct MessageContent
{
    pid_t pid;
    int key;
};

struct MessageFrame
{
    long msg_type;
    MessageContent msg_content;
};

int send_msg(int msgid, MessageFrame msg, int msgflg);
ssize_t receive_msg(int msg_id, MessageFrame *msgp, int msgflg);
std::vector<std::string> split(const std::string &str, char delimiter = ' ');
char digit_to_char(int num);
bool shut_down_msg(int msg_id);

#endif // UTIL_H
