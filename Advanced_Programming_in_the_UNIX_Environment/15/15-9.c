#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#define MAXLINE 50

static void sig_pipe(int);

int main(int argc,char *argv[])
{
	int n,fd1[2],fd2[2];
	pid_t pid;
	char line[MAXLINE];

	if(signal(SIGPIPE,sig_pipe)==SIG_ERR)
		printf("signal error!\n");
	if(pipe(fd1)<0||pipe(fd2)<0)
		printf("pipe error!\n");

	if((pid=fork())<0)
		printf("fork error!\n");
	else if(pid>0)
	{
		close(fd1[0]);
		close(fd2[1]);
		while(fgets(line,MAXLINE,stdin)!=NULL)
		{
			n=strlen(line);
			if(write(fd1[1],line,MAXLINE)!=n)
				printf("write error to pipe!\n");
			if((n=read(fd2[0],line,MAXLINE))<0)
				printf("read error from pipe!\n");
			if(n==0)
			{
				printf("child closed pipe!\n");
				break;
			}
			line[n]=0;
			if(fputs(line,stdout)==EOF)
				printf("fputs error!\n");
		}
		if(ferror(stdin))
			printf("fgets error on stdin!\n");

		exit(0);
	}
	else
	{
		close(fd1[1]);
		close(fd2[0]);
		if(fd1[0]!=STDIN_FILENO)
		{
			if(dup2(fd1[0],STDIN_FILENO)!=STDIN_FILENO)
				printf("dup2 error to stdin!\n");
			close(fd1[0]);
		}
		if(fd2[1]!=STDOUT_FILENO)
		{
			if(dup2(fd2[1],STDOUT_FILENO)!=STDOUT_FILENO)
				printf("dup2 error to stdout!\n");
			close(fd2[1]);
		}

		if(execl("./15-8","15-8",(char *) 0)<0)
			printf("execl error!\n");
	}

	exit(0);
}

static void sig_pipe(int signo)
{
	printf("SIGPIPE caught!\n");
	exit(1);
}
