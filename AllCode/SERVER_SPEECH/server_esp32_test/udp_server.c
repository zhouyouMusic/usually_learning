#include <stdio.h>
#include <string.h>
#include <unistd.h> // for close()
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 12347  
#define MAXDATASIZE 1024  

int main()
{

    int sockefd; // socket descriptors

    struct sockaddr_in server; //server's address information
    struct sockaddr_in client;  // client's address information

    socklen_t sin_size;

    int num, i;

    char recemsg[MAXDATASIZE];
    char sendmsg[MAXDATASIZE];
    char condition[] = "quit";

    /*create udp socket*/
    if((sockefd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        printf("create socket failed\n");
        exit(1);

	}

    bzero(&server, sizeof(server));//将字节类型的字符串的前n个字节为零，包括'\0'
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sockefd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        printf("bind error!\n");
    }

    sin_size = sizeof(struct sockaddr_in);

    while(1)
    {
        num = recvfrom(sockefd, recemsg, MAXDATASIZE, 0, (struct sockaddr *) &client, &sin_size);

        if(num < 0)
        {
            printf("recvfrome error\n");
        }

        recemsg[num] = '\0';
        printf("you got a message (%s) from %s\n", recemsg, inet_ntoa(client.sin_addr));

	     sendto(sockefd, "my name is JsonBoEn",30 ,0,(struct sockaddr *)&client,sin_size);
    }

    close(sockefd);

    return 0;
}
