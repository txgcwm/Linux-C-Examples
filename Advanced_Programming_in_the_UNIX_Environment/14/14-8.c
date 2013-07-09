#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc,char *argv[])
{
	int i,fd;

	for(i=1;i<argc;i++)
	{
		if((fd=open(argv[i],O_RDONLY))<0)
		{
			printf("%s:can not open\n",argv[i]);
			continue;
		}

		if(isastream(fd)==0)
			printf("%s:not a stream\n",argv[i]);
		else
			printf("%s:stream device\n",argv[i]);
	}

	exit(0);
}
