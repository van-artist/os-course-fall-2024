#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>

#define SHMKEY 75 /*定义共享区关键词*/

int shmid, p1, p2;
int *addr;

void CLIENT()
{
	int i;
	shmid = shmget(SHMKEY, 1024, 0777); /*获取共享区，长度为1024，关键词SHMKEY*/
	addr = (int *)shmat(shmid, 0, 0);	/*共享区起始地址为addr*/
	for (i = 9; i >= 0; i--)
	{
		while (*addr != -1)
			;
		printf("(client)sent%d\n", i); /*打印(client)sent*/
		*addr = i;					   /*把i赋给addr*/
	}
	exit(0);
}

void SERVER()
{
	shmid = shmget(SHMKEY, 1024, 0777 | IPC_CREAT); /*创建共享区*/
	addr = (int *)shmat(shmid, 0, 0);				/*共享区起始地址为addr*/
	do
	{
		*addr = -1;
		while (*addr == -1)
			;
		printf("(server)received %d\n", *addr); /*服务进程使用共享区*/
	} while (*addr);
	shmctl(shmid, IPC_RMID, 0);
	exit(0);
}

int main()
{

	while ((p1 = fork()) == -1)
		; /*创建子进程p1,失败时循环*/
	if (!p1)
		SERVER(); /*子进程p1执行SERVER()*/
	while ((p2 = fork()) == -1)
		; /*创建子进程p2,失败时循环*/
	if (!p2)
		CLIENT(); /*子进程p2执行CLIENT()*/
	wait(0);
	wait(0);
	return 0;
}