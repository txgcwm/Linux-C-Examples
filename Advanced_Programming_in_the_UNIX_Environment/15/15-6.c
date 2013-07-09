#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc,char * argv[])
{
	int c;

	while((c=getchar())!=EOF)
	{
		if(isupper(c))
			c=tolower(c);
		if(putchar(c)==EOF)
			printf("output error!\n");

		if(c=='\n')
			fflush(stdout);
	}
	exit(0);
}
