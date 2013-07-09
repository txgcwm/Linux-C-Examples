#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#if defined(BSD)||defined(MACOS)
	#include <sys/socket.h>
	#include <netinet/in.h>
#endif


void print_family(struct addrinfo *aip)
{
	printf("family:");

	switch(aip->ai_family)
	{
		case AF_INET:
			printf("inet\n");
			break;
		case AF_INET6:
			printf("inet6\n");
			break;
		case AF_UNIX:
			printf("unix\n");
			break;
		case AF_UNSPEC:
			printf("unspecified\n");
			break;
		default:
			printf("unknown\n");
			break;
	}
}

void print_type(struct addrinfo *aip)
{
	printf("type:");
	switch(aip->ai_socktype)
	{
		case	SOCK_STREAM:
			printf("stream\n");
			break;
		case	SOCK_DGRAM:
			printf("datagram\n");
			break;
		case	SOCK_SEQPACKET:
			printf("seqpacket\n");
			break;
		case	SOCK_RAW:
			printf("raw\n");
			break;
		default:
			printf("unknown (%d)\n",aip->ai_socktype);
			break;
	}
}

void print_protocol(struct addrinfo *aip)
{
	printf("protocol:");
	switch(aip->ai_protocol)
	{
		case	0:
			printf("default\n");
			break;
		case	IPPROTO_TCP:
			printf("TCP\n");
			break;
		case	IPPROTO_UDP:
			printf("UDP\n");
			break;
		case	IPPROTO_RAW:
			printf("raw\n");
			break;
		default:
			printf("unknown (%d)\n",aip->ai_protocol);
			break;
	}
}

void print_flags(struct addrinfo *aip)
{
	printf("flags:");
	if(aip->ai_flags==0)
		printf("0");
	else
	{
		if(aip->ai_flags & AI_PASSIVE)
			printf(" passive\n");
		if(aip->ai_flags & AI_CANONNAME)
			printf(" canon\n");
		if(aip->ai_flags & AI_NUMERICHOST)
			printf(" numhost\n");
	#if defined(AI_NUMERICSERV)
		if(aip->ai_flags & AI_NUMERICSERV)
			printf(" numserv\n");
	#endif
	#if defined(AI_V4MAPPED)
		if(aip->ai_flags & AI_V4MAPPED)
			printf(" v4mapped\n");
	#endif
	#if defined(AI_ALL)
		if(aip->ai_flags & AI_ALL)
			printf(" all\n");
	#endif
	}
}

int main(int argc,char *argv[])
{
	struct addrinfo *ailist,*aip;
	struct addrinfo hint;
	struct sockaddr_in *sinp;
	const char *addr;
	int err;
	char abuf[INET_ADDRSTRLEN];

	if(argc!=3)
		printf("usage:%s nodename service\n",argv[0]);

	hint.ai_flags=AI_CANONNAME;
	hint.ai_family=0;
	hint.ai_socktype=0;
	hint.ai_protocol=0;
	hint.ai_addrlen=0;
	hint.ai_canonname=NULL;
	hint.ai_addr=NULL;
	hint.ai_next=NULL;

	if((err=getaddrinfo(argv[1],argv[2],&hint,&ailist))!=0)
		printf("getaddrinfo error:%s\n",gai_strerror(err));
	for(aip=ailist;aip!=NULL;aip=aip->ai_next)
	{
		print_flags(aip);
		print_family(aip);
		print_type(aip);
		print_protocol(aip);
		printf("\n\thost %s\n",aip->ai_canonname?aip->ai_canonname:"-");

		if(aip->ai_family==AF_INET)
		{
			sinp=(struct sockaddr_in *)aip->ai_addr;
			addr=inet_ntop(AF_INET,&sinp->sin_addr,abuf,INET_ADDRSTRLEN);
			printf("address %s\n",addr?addr:"unknown");
			printf("port %d",ntohs(sinp->sin_port));
		}

		printf("\n");
	}
	exit(0);
}



