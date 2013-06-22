#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "tcp_scan.h"

int info(const struct sockaddr *sockaddr, socklen_t addrlen, int state,
		 void *arg)
{
	struct sockaddr_in *sin = (struct sockaddr_in *)sockaddr;
	printf("%s:%d ", inet_ntoa(sin->sin_addr), ntohs(sin->sin_port));

	switch (state) {
	case TS_ACCEPTED:
		printf("Connection accepted\n");
		break;
	case TS_REFUSED:
		printf("Connection refused\n");
		break;
	case TS_UNREACH:
		printf("Destination unreachable\n");
		break;
	}

	return 0;
}

int main(int argc, char **argv)
{
	struct addrseg addrseg[2];
	struct in_addr inaddr[2];
	unsigned short ports[] = {80, 21, 0};

	addrseg[0].as_family = AF_INET;
	addrseg[0].as_address = inaddr;

	inet_pton(AF_INET, "162.105.0.0", addrseg[0].as_address);
	addrseg[0].as_bits = 16;
	addrseg[0].as_next = addrseg + 1;
	addrseg[1].as_family = AF_INET;
	addrseg[1].as_address = inaddr + 1;
	inet_pton(AF_INET, "166.111.0.0", addrseg[1].as_address);
	addrseg[1].as_bits = 16;
	addrseg[1].as_next = NULL;

	tcp_scan(addrseg, ports, 0, "eth0", 1, info, NULL);

	return 0;
}
