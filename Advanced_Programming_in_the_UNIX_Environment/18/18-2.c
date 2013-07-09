#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
	struct termios term;

	if(tcgetattr(STDIN_FILENO,&term)<0)
		printf("tcgetattr error!\n");

	switch(term.c_cflag & CSIZE)
	{
		case CS5:
			printf("5 bits/bytes\n");
			break;
		case CS6:
			printf("6 bits/bytes\n");
			break;
		case CS7:
			printf("7 bits/bytes\n");
			break;
		case CS8:
			printf("8 bits/bytes\n");
			break;
		default:
			printf("unknown bits/bytes\n");
			break;
	}

	term.c_cflag &=~CSIZE;
	term.c_cflag |=CS8;

	if(tcsetattr(STDIN_FILENO,TCSANOW,&term)<0)
		printf("tcsetattr error!\n");

	exit(0);
}
