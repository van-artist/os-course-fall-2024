#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <signal.h>

#define MSGKEY 75

struct msgform
{
	long mtype;
	char mtext[256];
} msg;
int msgqid;
main()
{
	int i, pid, *pint;
	extern cleanup();
	for (i = 0; i < 20; i++) /*软中断处理*/
		signal(i, cleanup);

	if ((msgqid = msgget(MSGKEY, 0777 | IPC_CREAT | IPC_EXCL)) == -1) /*建立消息队列失败*/
	{
		printf("This message queue already exists. \n", *pint);
		return;
	}
	for (;;)
	{
		msgrcv(msgqid, &msg, 256, 1, 0); /*接收来自客户进程的消息*/
		pint = (int *)msg.mtext;
		pid = *pint;
		printf("server:receive from pid %d\n", pid);
		msg.mtype = pid;
		*pint = getpid();
		msgsnd(msgqid, &msg, sizeof(int), 0); /*发送应答消息msg*/
	}
}

cleanup()
{
	msgctl(msgqid, IPC_RMID, 0);
	exit(0);
}
