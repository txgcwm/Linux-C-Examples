#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

char buf[500000];

void set_fl(int fd,int flags)
{
	int val;

	if((val=fcntl(fd,F_GETFL,0))<0)
		printf("fcntl F_GETFL error");

	val |=flags;

	if(fcntl(fd,F_SETFL,val)<0)
		printf("fcntl F_SETFL error");
}

void clr_fl(int fd,int flags)
{
	int val;

	if((val=fcntl(fd,F_GETFL,0))<0)
		printf("fcntl F_GETFL error");

	val &=~flags;

	if(fcntl(fd,F_SETFL,val)<0)
		printf("fcntl F_SETFL error");
}

int main(char argc,char **argv)
{
	int ntowrite,nwrite;
	char *ptr;

	ntowrite=read(STDIN_FILENO,buf,sizeof(buf));
	fprintf(stderr,"read %d bytes\n",ntowrite);

	set_fl(STDIN_FILENO,O_NONBLOCK);

	ptr=buf;
	while(ntowrite>0)
	{
		errno=0;
		nwrite=write(STDOUT_FILENO,ptr,ntowrite);
		fprintf(stderr,"nwrite=%d,errno=%d\n",nwrite,errno);

		if(nwrite>0)
		{
			ptr+=nwrite;
			ntowrite -=nwrite;
		}
	}

	clr_fl(STDIN_FILENO,O_NONBLOCK);
	exit(0);
}
