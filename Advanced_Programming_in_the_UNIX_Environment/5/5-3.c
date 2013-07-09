#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void print_stdio(const char *,FILE *);

int main(int argc,char * argv[])
{
	FILE *fp;
	fputs("enter any character:\n",stdout);

	if(getchar()==EOF)
		printf("getchar error!\n");

	fputs("one line to standard error!\n",stderr);

	print_stdio("stdin",stdin);
	print_stdio("stdout",stdout);
	print_stdio("stderr",stderr);

	if((fp=fopen("/etc/motd","r"))==NULL)
		printf("open the file error!\n");

	if(getc(fp)==EOF)
		printf("getc error!\n");

	print_stdio("/etc/motd",fp);

	exit(0);
}

void print_stdio(const char * name,FILE * fp)
{
	printf("stream = %s :",name);

	if(fp->_IO_file_flags & _IO_UNBUFFERED)
		printf("unbuffered\n");
	else if(fp->_IO_file_flags & _IO_LINE_BUF)
		printf("line buffered\n");
	else
		printf("fully buffered\n");

	printf("buffer size = %d \n",fp->_IO_buf_end-fp->_IO_buf_base);
}


