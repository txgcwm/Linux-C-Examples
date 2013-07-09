#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	printf("hello the world from process ID %d\n",getpid());
	exit(0);
}
