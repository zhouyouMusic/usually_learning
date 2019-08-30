#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <unistd.h>
#include <netdb.h>
#include <ctype.h>
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/ioctl.h>
#include <alsa/asoundlib.h>
#include <linux/soundcard.h>
#include <arpa/inet.h>


int socketCreate(char * ipAddress,int port)
{
	int uSocket = 0;
	uSocket = socket(AF_INET,SOCK_STREAM,0);
	if(uSocket < 0)
	{
		perror(" failed to create socket");
		return -1;
	}
	struct sockaddr_in sockAddr;
	memset(&sockAddr,0,sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(port);
	if(inet_pton(AF_INET,ipAddress,&sockAddr.sin_addr) <= 0)
	{
		perror( "failed to convert 	IPADDR to internet IP");
		return -1;
	}	
	if(connect(uSocket,(struct sockaddr *)&sockAddr,sizeof(struct sockaddr_in)) < 0)
	{
		perror("failed to connect to the server");
		return -1;
	}
	return uSocket;
}

int main()
{
	char *ttsBuf = "GET http://tts.baidu.com/text2audio?cuid=48-5A-B6-47-0A-BB&lan=zh&ctp=1&pdt=90&tex=""你好，小朋友，我是你的好朋友小哈"" HTTP/1.1\r\nHOST: tts.baidu.com\r\nConnection: close\r\nAccept: */*\r\n";

	struct hostent *ttsHost;
	ttsHost = gethostbyname("tts.baidu.com");	
	//	printf("address type: %s \n",(ttsHost->h_addrtype==AF_INET)?"AF_INET":"AF_INET6");
	char * ipAddr = inet_ntoa(*(struct in_addr*)ttsHost->h_addr_list[0]);
	int ttsSockfd = socketCreate(ipAddr,80);
	int retVal = write(ttsSockfd,ttsBuf,strlen(ttsBuf));
	printf("%s",ttsBuf);
	int length = 0;
	FILE * fp = fopen("tts.mp3","wb");
	char line[1024];
	printf("sock	%d\n",ttsSockfd);
	while ((length = read(ttsSockfd,line,1)) == 1)
	{
		printf("%c",line[0]);
		fwrite(line,1,1,fp);
	}	
}
