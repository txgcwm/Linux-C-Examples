#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char *env_init[]={"USER=unknown","PATH=/tmp",NULL};

int main(int argc,char * argv[])
{
	pid_t pid;

	if((pid=fork())<0)
		printf("fork error!\n");
	else if(pid==0)
	{
		if(execle("/home/sar/bin/echoall","echoall","myarg1","MY ARG2",(char *)0,env_init)<0)	
			printf("execle error!\n");
	}
	
	if(waitpid(pid,NULL,0)<0)
		printf("wait error!\n");

	if((pid=fork())<0)
		printf("fork error!\n");
	else if(pid==0)
	{
		if(execlp("echoall","echoall","only 1 arg",(char *)0)<0)	
			printf("execlp error!\n");
	}

	exit(0);
}



