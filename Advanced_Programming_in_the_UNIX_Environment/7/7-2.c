#include <stdio.h>
#include <stdlib.h>

static void my_exit1(void);
static void my_exit2(void);

int main(int argc,char * argv[])
{
	if(atexit(my_exit2)!=0)
		printf("can not register my_exit2!\n");
	if(atexit(my_exit1)!=0)
		printf("can not register my_exit1!\n");
	if(atexit(my_exit1)!=0)
		printf("can not register my_exit1!\n");

	printf("main is done!\n");
	return 0;
}

static void my_exit1(void)
{
	printf("1:first exit handler!\n");
}

static void my_exit2(void)
{
	printf("2:second exit handler!\n");
}

