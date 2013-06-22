#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/sendfile.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/stat.h>

#define SER_PORT 10500
#define MAX_LEN 512

int main(int argc,char **argv)
{
    int sd;
    int rn;
    struct sockaddr_in client,server;
    struct hostent *h;
    int fq;
    int i;
    char ch;
    struct stat st;
    char *filename = "2.jpg";
    int len = 0;

    sd=socket(AF_INET,SOCK_STREAM,0);
    if(sd < 0)
    {
        printf( "Creating socket error!\n ");
        exit(1);
    }

    if(argc < 2)
    {
        printf( "Not enough parameter!\n ");
        exit(1);
    }

    h=gethostbyname(argv[1]);
    if(h == NULL)
    {
        printf( "Can 't get hostname ");
        exit(1);
    }

    bzero(&server,sizeof(server));
    server.sin_family=h-> h_addrtype;
    server.sin_port=htons(SER_PORT);
    server.sin_addr = *((struct in_addr *)h-> h_addr);

    int opt=1;
    setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    if(connect(sd,(struct sockaddr *)&server,sizeof(server)) < 0)
    {
        printf( "Connect Error!\n ");
        exit(1);
    }

    fq = open(filename , O_RDONLY);
    if( fq < 0 )
    {
        perror("file error");
        exit(1);
    }

    stat(filename,&st);//获取文件大小
    len = st.st_size;

    if(sendfile(sd,fq,0,len) < 0)
    {
        perror("send error");
        exit(1);
    }

    close(sd);
    close(fq);
    return 0;
}
