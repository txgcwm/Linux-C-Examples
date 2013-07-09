#include <stdio.h>
#include <fcntl.h>
//#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


int main(int argc,char * argv[])
{
	if(open("tempfile",O_RDWR)<0)
		printf("open the file error!\n");

	if(unlink("tempfile")<0)
		printf("unlink the file error!\n");
	else
		printf("file unlinked!\n");

	sleep(15);
	printf("done\n");

	_exit(0);
}

