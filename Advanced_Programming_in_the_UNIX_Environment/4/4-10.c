#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef SOLARIS
	#include <sys/mkdev.h>
#endif

int main(int argc,char * argv[])
{
	int i;
	struct stat buf;

	for(i=1;i<argc;i++)
	{
		printf("%s:\n",argv[i]);
		if(stat(argv[i],&buf)<0)
		{
			printf("stat error!\n");
			continue;
		}

		printf("dev = %d/%d\n",major(buf.st_dev),minor(buf.st_dev));

		if(S_ISCHR(buf.st_mode)||S_ISBLK(buf.st_mode))
		{
			printf("(%s) rdev = %d/%d\n",
				(S_ISCHR(buf.st_mode)) ? "character":"block",
				major(buf.st_rdev),minor(buf.st_rdev));
		}
		else 
			printf("other file!\n");
	}

	printf("\n");

	_exit(0);
}

