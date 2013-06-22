#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <error.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SER_PORT 10500
#define MAX_LEN 512

int main(int argc,char **argv)
{
    int listensd,connectsd;
    struct sockaddr_in server;
    struct sockaddr_in client;
    char buf[MAX_LEN];
    FILE *fp;
    int i;
    char *filename = "2new.jpg ";

    listensd=socket(AF_INET,SOCK_STREAM,0);
    if(listensd < 0)
    {
        perror( "Creating socket failed: ");
        exit(1);
    }

    int opt=1;
    setsockopt(listensd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    bzero(&server,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(SER_PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(listensd,(struct sockaddr *)&server,sizeof(server)) < 0)
    {
        perror( "Bind error. ");
        exit(1);
    }

    if(listen(listensd,5) == -1)
    {
        perror( "listen() error\n ");
        exit(1);
    }

    fp=fopen( filename, "ab");
    if(fp == NULL)
    {
        printf( "File open error!\n ");
        exit(1);
    }

    int rn ;
    int sin_len=sizeof(client);
    if((connectsd = accept(listensd,(struct sockaddr *)&client,&sin_len)) < 0)
    {
        printf( "Can 't accept the request!\n ");
        exit(1);
    }

    while(1)
    {
        rn=recv(connectsd,buf,MAX_LEN,0);
        if(rn < 0)
        {
            printf( "Can 't receive file!\n ");
            exit(1);
        }

        buf[rn]= '\0 ';

        if(rn != 0)
        {
            fwrite(buf,1,512,fp);
            bzero(buf,sizeof(buf));
        }
        else
        {
            printf("receive over.\n");
            break;
        }
    }
    close(connectsd);
    fclose(fp);
    close(listensd);
    return 0;
}
