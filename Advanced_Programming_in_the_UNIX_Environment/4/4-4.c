#include <stdio.h>
#include <fcntl.h>
//#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define RWRWRW ()

int main(int argc,char * argv[])
{
	struct stat statbuf;

	if(stat("foo",&statbuf)<0)
		printf("get the stat error\n");
	if(chmod("foo",(statbuf.st_mode&~S_IXGRP)|S_ISGID)<0)
		printf("chmod error for foo!\n");

	if(chmod("bar",S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)<0)
		printf("chmod error for bar!\n");

	_exit(0);
}

