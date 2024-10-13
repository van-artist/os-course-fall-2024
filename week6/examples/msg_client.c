#include <stdio.h>
#include <sys/msg.h>
#define MSGKEY 75 /*定义关键词*/

struct msgform
{
	long mtype;
	char mtext[256];
};

main()
{
	struct msgform msg;
	int msgqid, pid, *pint;

	if ((msgqid = msgget(MSGKEY, 0777)) == -1) /*建立消息队列失败*/
	{
		printf("This message queue does not exist. \n", *pint);
		return;
	}
	pid = getpid();
	pint = (int *)msg.mtext;
	*pint = pid;
	msg.mtype = 1;						  /*指定消息类型*/
	msgsnd(msgqid, &msg, sizeof(int), 0); /*往msgqid发送消息msg*/
	msgrcv(msgqid, &msg, 256, pid, 0);	  /*接收来自服务器进程的消息*/
	printf("client:receive from pid %d\n", *pint);
}
