#include <stdio.h>
#include <fcntl.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <unistd.h>

int main(int argc,char * argv[])
{
	int i;

	if(argc<2)
		printf("usage:4-2 <pathname>");

	for(i=1;i<argc;i++)
	{
		if(access(argv[i],R_OK)<0)
			printf("access error for %s\n",argv[i]);
		else
			printf("read access OK!\n");

		if(open(argv[i],O_RDONLY)<0)
			printf("open error for %s\n",argv[i]);
		else
			printf("open for reading OK!\n");

		printf("\n");
	}

	_exit(0);
}

