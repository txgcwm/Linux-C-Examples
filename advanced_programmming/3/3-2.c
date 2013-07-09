#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

char buf1[]="abcdefghij";
char buf2[]="ABCDEFGHIJ";

int main(int argc,char * argv[])
{
	int fd;
	if((fd=creat("file.hole",O_RDWR))<0)
		printf("error!\n");

	if(write(fd,buf1,10)!=10)
		printf("error!\n");

	if(lseek(fd,1024,SEEK_SET)==-1)
		printf("error!\n");

	if(write(fd,buf2,10)!=10)
		printf("error!\n");

	_exit(0);
}
