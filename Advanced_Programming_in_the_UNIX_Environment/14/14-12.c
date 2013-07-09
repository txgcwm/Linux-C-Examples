#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FILE_MODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

int main(int argc,char *argv[])
{
	int fdin,fdout;
	void *src,*dst;
	struct stat statbuf;

	if(argc!=3)
		printf("usage: %s <fromfile> <tofile>",argv[0]);

	if((fdin=open(argv[1],O_RDONLY))<0)
		printf("can not open %s for reading",argv[1]);

	if((fdout=open(argv[2],O_RDWR|O_CREAT|O_TRUNC,FILE_MODE))<0)
		printf("can not creat %s for writing",argv[2]);

	if(fstat(fdin,&statbuf)<0)
		printf("fstat error!\n");

	if(lseek(fdout,statbuf.st_size-1,SEEK_SET)==-1)
		printf("lseek error!\n");
	if(write(fdout," ",1)!=1)
		printf("write error!\n");

	if((src=mmap(0,statbuf.st_size,PROT_READ,MAP_SHARED,fdin,0))==MAP_FAILED)
		printf("mmap error for input!\n");

	if((dst=mmap(0,statbuf.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fdout,0))==MAP_FAILED)
		printf("mmap error for output!\n");

	memcpy(dst,src,statbuf.st_size);

	exit(0);
}
