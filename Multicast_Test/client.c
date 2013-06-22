#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>

#define PORT 8888
#define MCAST "224.0.0.88"

int main(int argc, char *argv[])
{
	int s;
	int ret;
	int size;
	int ttl = 1;
	int loop = 0;
	char buffer[1024];
	struct ip_mreq mreq;
	struct sockaddr_in localaddr, fromaddr;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket error");
		return -1;
	}

	localaddr.sin_family = AF_INET;
	localaddr.sin_port = htons(PORT);
	localaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = bind(s, (struct sockaddr *)&localaddr, sizeof(localaddr));
	if (ret < 0) {
		perror("bind error");
		return -1;
	}

	if (setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
		perror("IP_MULTICAST_TTL");
		return -1;
	}

	if (setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)) < 0) {
		perror("IP_MULTICAST_LOOP");
		return -1;
	}

	mreq.imr_multiaddr.s_addr = inet_addr(MCAST);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
		perror("IP_ADD_MEMBERSHIP");
		return -1;
	}

	int len = sizeof(fromaddr);
	while(1) {
		sleep(1);
		memset(buffer, 0, sizeof(buffer));
		size = recvfrom(s, buffer, 1024, 0, (struct sockaddr *)&fromaddr, &len);
		if (size < 0) {
			perror("recvfrom ");
			return -1;
		}
		printf("receive message:%s\nPort is:%d\n", buffer, fromaddr.sin_port);
		size = sendto(s, "OK", 2, 0, (struct sockaddr *)&fromaddr,sizeof(fromaddr));
	}

	ret = setsockopt(s, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));
	if (ret < 0) {
		perror("IP_DROP_MEMBERSHIP");
		return -1;
	}
	close(s);

	return 0;
}



