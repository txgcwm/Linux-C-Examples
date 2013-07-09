#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(void)
{
	struct termios term;
	long vdisable;

	if(isatty(STDIN_FILENO)==0)
		printf("standard input is not a terminal device");

	if((vdisable=fpathconf(STDIN_FILENO,_PC_VDISABLE))<0)
		printf("fpathconf error or _POSIX_VDISABLE not in effect\n");

	if(tcgetattr(STDIN_FILENO,&term)<0)
		printf("tcgetattr error\n");

	term.c_cc[VINTR]=vdisable;
	term.c_cc[VEOF]=2;

	if(tcsetattr(STDIN_FILENO,TCSAFLUSH,&term)<0)
		printf("tcsetattr error!\n");

	exit(0);
}
