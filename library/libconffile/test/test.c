#include <stdio.h>
#include "conffile.h"

int main(int argc,char **argv)
{
	char value[20];

	if(argc < 2)
	{
		printf("usage:./test filename\n");
		return -1;
	}

	write_conf_value("IP_SERVER", "192.168.1.101", argv[1]);

	read_conf_value("IP", value, argv[1]);
	printf("value: %s\n",value);

	return 0;
}


