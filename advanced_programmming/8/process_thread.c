#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/shm.h>

#define MAXLENTH  16
//#define GET_RELATE_ID

struct msg_buf
{
	int mtype;
	char data[255];
};

//get the relate id of the current process
void get_pid_uid_gid(void)
{
	printf("pid is: %d\n",getpid());
	printf("parent pid is: %d\n",getppid());
	
	printf("uid is: %d\n",getuid());
	printf("enable uid is:%d\n",geteuid());

	printf("gid is: %d\n",getgid());
	printf("enable gid is: %d\n",getegid());
	
	printf("\n");
	return;
}

void * pthread_first(void *parm)
{
	printf("_______________________________first thread have create____________________________________\n");
	return (void *)0;
}

void * pthread_second(void *parm)
{
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~second thread have create~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	return (void *)0;
}

int main(int argc,char * argv[])
{
	pid_t pid;   //for unnamed pipe
	int filedes[2];
	int read_num;
	char buffer[MAXLENTH];
	
	key_t key;   //for message
	int msgid;
	int ret;
	struct msg_buf msgbuf;
	
	char  * father_add,* son_add;  //for memory share
	int shmid;
	
	pthread_t tid1,tid2;   //for save thread id
	
	//creat a pipe to IPC for parent and son process
	if(pipe(filedes)==-1)
	{
		printf("creat a pipe error!\n");
		_exit(EXIT_FAILURE);
	}
	
	//creat a message queue 
	key=ftok("/tmp/2",'a');
	printf("key=[%x]\n",key);
	msgid=msgget(key,IPC_CREAT|0666);
	if(msgid==-1)
	{
		printf("create error\n");
		return -1;
	}
	
	//creat a memory share
	if((shmid=shmget(IPC_PRIVATE,1024,S_IRUSR|S_IWUSR))==-1)
	{
		fprintf(stderr,"create share memory error:%s\n\a",strerror(errno));
		exit(1);
	}

	pid=fork(); //the order to excute the parent process or son process is not sure,should use some method to synchronization

	if(pid>0)
	{
		printf("\n");
		printf("the parent process message:\n");
		
		//get pid uid gid etc
		#ifdef GET_RELATE_ID
			get_pid_uid_gid();
		#endif
		
		//use pipe to communication
		close(filedes[0]);//close the fd which we not use
		printf("start to write pipe......\n");
		if(write(filedes[1],"hello the word!\n",MAXLENTH)!=MAXLENTH)
		{
			printf("write to pipe error!");
			_exit(EXIT_FAILURE);
		}
		printf("^^^^^^^^^^^^finish to write pipe^^^^^^^^^^^\n");
		close(filedes[1]);
		
		//parent process send message to queue
		msgbuf.mtype=0;
		strcpy(msgbuf.data,"test haha");
		ret=msgsnd(msgid,&msgbuf,sizeof(msgbuf.data),IPC_NOWAIT);
		if(ret==-1)
		{
			printf("send message error\n");
			return -1;
		}
		
		//parent process:write something to memory share
		father_add=shmat(shmid,0,0);
		memset(father_add,0,1024);
		strncpy(father_add,"parent process share the information to son process!",1024);
		
		//parent thread creat a thread 
		ret=pthread_create(&tid1,NULL,(void *)pthread_first,NULL);
		if(ret)
		{
			printf("creat a thread error!\n");
			return 1;
		}
		//pthread_join(tid1,NULL);
		
		wait(NULL);
		printf("\n");
		_exit(EXIT_SUCCESS);
	}
	else if(pid==0)
	{
		sleep(5);//to ensure the parent process execute first
		printf("\n");
		printf("the son process message:\n");
		
		//get pid uid gid etc
		#ifdef GET_RELATE_ID
			get_pid_uid_gid();
		#endif
		
		//use pipe to communication
		close(filedes[1]);
		printf("start to read pipe......\n");
		read_num=read(filedes[0],buffer,MAXLENTH);
		printf("finish to read pipe,the read_num is %d\n",read_num);
		write(STDOUT_FILENO,buffer,read_num);
		close(filedes[0]);
		
		//son process recevice message from queue
		memset(&msgbuf,0,sizeof(msgbuf));
		ret=msgrcv(msgid,&msgbuf,sizeof(msgbuf.data),0,IPC_NOWAIT);
		if(ret==-1)
		{
			printf("recv message error\n");
			return -1;
		}
		printf("receive msg =[%s]\n",msgbuf.data);
		
		//son process:receive something from memory share
		son_add=(char*)shmat(shmid,0,0);
		printf("son process get the information is : %s\n",son_add);
		
		//son process creat a thread
		ret=pthread_create(&tid2,NULL,(void *)pthread_second,NULL);
		if(ret)
		{
			printf("creat a thread error!\n");
			return 1;
		}
		//pthread_join(tid2,NULL);
		
		printf("\n");
		sleep(5);
		_exit(EXIT_SUCCESS);
	}
	else
	{
		printf("creat the process error,please check it!\n");
		_exit(EXIT_FAILURE);
	}
	_exit(EXIT_SUCCESS);
}
