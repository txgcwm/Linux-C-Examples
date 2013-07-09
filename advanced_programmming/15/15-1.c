#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>


#define MAXLINE 50

int main(int argc,char * argv[])
{
	int n;
	int fd[2];
	pid_t pid;
	char line[MAXLINE];

	if(pipe(fd)<0)
		printf("creat the pipe error!\n");

	if((pid=fork())<0)
		printf("fork error!\n");
	else if(pid>0)
	{
		close(fd[0]);
		write(fd[1],"hello the world!\n",20);
	}
	else
	{
		close(fd[1]);
		n=read(fd[0],line,MAXLINE);
		write(STDOUT_FILENO,line,n);
		printf("\n");
	}

	exit(0);
}
