#include <stdio.h>
#include <fcntl.h>
//#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>

int main(int argc,char * argv[])
{
	int i,fd;
	struct stat statbuf;
	struct utimbuf timebuf;

	for(i=1;i<argc;i++)
	{
		if(stat(argv[i],&statbuf)<0)
		{
			printf("%s:get stat error!\n",argv[i]);
			continue;
		}

		if((fd=open(argv[i],O_RDWR|O_TRUNC))<0)
		{
			printf("%s:open file error!\n",argv[i]);
			continue;
		}
		close(fd);

		timebuf.actime=statbuf.st_atime;
		timebuf.modtime=statbuf.st_mtime;

		if(utime(argv[i],&timebuf)<0)
		{
			printf("%s:utime file error!\n",argv[i]);
			continue;
		}
	}
	printf("done\n");

	_exit(0);
}

