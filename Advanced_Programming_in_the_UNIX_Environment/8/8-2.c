#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int glob=6;

int main(int argc,char * argv[])
{
	int var;
	pid_t pid;
	var=88;

	printf("before fork____\n");

	if((pid=fork())<0)
	{
		printf("fork error!\n");
	}
	else if(pid==0)
	{
		glob++;
		var++;
		_exit(0);
		//return 0;
	}
	

	printf("pid=%d,glob=%d,var=%d\n",getpid(),glob,var);

	exit(0);
}

