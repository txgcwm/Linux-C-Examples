#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int glob=6;
char buf[]="a write to stdout\n";

int main(int argc,char * argv[])
{
	int var;
	pid_t pid;
	var=88;

	if(write(STDOUT_FILENO,buf,sizeof(buf)-1)!=sizeof(buf)-1)
		printf("write error!\n");

	printf("before fork____\n");

	if((pid=fork())<0)
	{
		printf("fork error!\n");
	}
	else if(pid==0)
	{
		glob++;
		var++;
	}
	else
		sleep(2);

	printf("pid=%d,glob=%d,var=%d\n",getpid(),glob,var);
}
