#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>

#define MCAST_PORT 8888
#define MCAST_ADDR "224.0.0.88"

int main(int argc, char *argv[])
{
	int ret;
	int s;
	int i = 1;
	int size = 0;
	int len = 0;
	char buffer[1024];
	struct sockaddr_in Multi_addr;
	struct sockaddr_in client_addr;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket error");
		return -1;
	}
	Multi_addr.sin_family = AF_INET;
	Multi_addr.sin_port = htons(MCAST_PORT);
	Multi_addr.sin_addr.s_addr = inet_addr(MCAST_ADDR);
	len = sizeof(client_addr);

	for (;;) {
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%d", i);
		size = sendto(s, buffer, strlen(buffer), 0, (struct sockaddr *)&Multi_addr, sizeof(Multi_addr));
		if (size < 0) {
			perror("sendto error");
		}

		sleep(1);
		i++;
		memset(buffer, 0, sizeof(buffer));
		
		size = recvfrom(s, buffer, 1024, 0, (struct sockaddr *)&client_addr, &len);
		write(1, buffer, size);
	}
	close(s);

	return 0;
}



