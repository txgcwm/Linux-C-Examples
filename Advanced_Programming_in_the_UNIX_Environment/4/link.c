#include <stdio.h>
#include <fcntl.h>
//#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


int main(int argc,char * argv[])
{
	
	if(link("temp","tempfile")<0)
		printf("link the file error!\n");
	else
		printf("file linked!\n");

	sleep(3);
	printf("done\n");

	_exit(0);
}
