#include <stdio.h>
#include <setjmp.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
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
#include <sys/time.h>
#include <alsa/asoundlib.h>
#include <linux/soundcard.h>


int socketCreate(char * ipAddress,int port)
{
	int uSocket = 0;
	uSocket = socket(AF_INET,SOCK_STREAM,0);
	if(uSocket <= 0)
	{
		return -1;
	}
	struct sockaddr_in sockAddr;
	memset(&sockAddr,0,sizeof(struct sockaddr_in));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(port);
	if(inet_pton(AF_INET,ipAddress,&sockAddr.sin_addr) <= 0)
	{
		return -1;
	}	
	if(connect(uSocket,(struct sockaddr *)&sockAddr,sizeof(struct sockaddr_in)) < 0)
	{
		return -1;
	}
	return uSocket;
}


int main()
{
	char * writeBuf= "GET /v3/iot/management/schema?pageNo=1 HTTP/1.1\n"
				"Host:iothdm.gz.baidubce.com\n"
				"Authorization: bce-auth-v1/f81a9e15540d49d5a61905ee196b6fef/2017-10-10T21:08:54Z/1800/host/pkOkMMrssww1uAINT4om9FlhCPu6nLbHOt6+1f4l9SU=\r\n";
	
	struct hostent *ttsHost;
	ttsHost = gethostbyname("81c62603c7854e79b258b84dc4a15f24.mqtt.iot.gz.baidubce.com");	
		printf("address type: %s \n",(ttsHost->h_addrtype==AF_INET)?"AF_INET":"AF_INET6");
	char * ipAddr = inet_ntoa(*(struct in_addr*)ttsHost->h_addr_list[0]);
	
	printf("ipAddr	%s\n",ipAddr);
	
	return 0;
}

















