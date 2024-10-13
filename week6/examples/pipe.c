#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
int p1, p2;

main()
{
	int fd[2];
	char OutPipe[100], InPipe[100];
	pipe(fd); /*父进程建立管道*/
	while ((p1 = fork()) == -1)
		;		 /*创建子进程p1,失败时循环*/
	if (p1 == 0) /*执行子进程p1*/
	{
		lockf(fd[1], 1, 0); /*加锁锁定写入端*/
		sprintf(OutPipe, "child 1 process is sending message!");
		write(fd[1], OutPipe, 50); /*把OutPipe中的50个字符写入管道*/
		sleep(5);				   /*睡眠5秒*/
		lockf(fd[1], 0, 0);		   /*释放写入端*/
		exit(0);				   /*关闭pid1*/
	}
	else /*从父进程返回，执行父进程*/
	{
		while ((p2 = fork()) == -1)
			;		 /*创建子进程p2,失败时循环*/
		if (p2 == 0) /*由子进程p2返回，执行子进程pid2*/
		{
			lockf(fd[1], 1, 0); /*加锁锁定写入端*/
			sprintf(OutPipe, "child 2 process is sending message!");
			write(fd[1], OutPipe, 50); /*把OutPipe中的50个字符写入管道*/
			sleep(5);				   /*睡眠5秒*/
			lockf(fd[1], 0, 0);		   /*释放写入端*/
			exit(0);				   /*关闭pid2*/
		}
		else
		{
			read(fd[0], InPipe, 50); /*读取管道中的消息*/
			printf("%s\n", InPipe);	 /*打印消息*/
			read(fd[0], InPipe, 50);
			printf("%s\n", InPipe);
		}
	}
}