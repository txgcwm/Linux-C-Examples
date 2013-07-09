#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

void print_exit(int status)
{
	if(WIFEXITED(status))
		printf("normal termination,exit status = %d \n",WEXITSTATUS(status));
	else if(WIFSIGNALED(status))
		printf("abnormal termination,signal number =%d %s\n",WTERMSIG(status),
			#ifdef WCOREDUMP
				WCOREDUMP(status)? "(core file generated)":"");
			#else
				"");
			#endif
	else if(WIFSTOPPED(status))
		printf("child stopped,signal number = %d\n",WSTOPSIG(status));
}

int main(int argc,char *argv[])
{
	int status;

	if((status=system("date"))<0)
		printf("system error!\n");
	print_exit(status);

	if((status=system("nosuchcommand"))<0)
		printf("system error!\n");
	print_exit(status);

	if((status=system("who;exit 44"))<0)
		printf("system error!\n");
	print_exit(status);
	
	return(status);
}
