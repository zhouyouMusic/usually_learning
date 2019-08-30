#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<string.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<pthread.h>
#include<signal.h>
#include<time.h>
#include<errno.h>
#include<memory.h>
#include<unistd.h>
#include<netdb.h>
#include "android_recognition.h"
#include "huwenvadInterface.h"
#include "voiceInterface.h"


static void micShortToFloat(short *input,float *output,float *aec,int nMics,int length) 
{
	int k = 0;
   	int j = 0;
        //        int length = (FRAME_LEN << 1);
    float *aec2 = aec + 1024;
    for (k = 0; k < length;k++) 
	{
    	output[k] = input[8 + 12 * k] * 0.00003052f;
		aec[k] = input[10 + 12 * k] * 0.00003052f;
		aec2[k] = input[11 + 12 * k] * 0.00003052f;
	}
	for (; j < nMics - 1;j++) 
	{
		for (k = 0; k < length;k++) 
		{
			output[k + (j + 1) * length] = input[j + 12 * k] * 0.00003052f;
        }
	}
}

int writeRecStart(int sockfd)
{
	struct post_content sendData = {0};
	sendData.contType = 0;
	return write(sockfd,&sendData,sizeof(struct post_content));
}
int writeRecStop(int sockfd)
{
	struct post_content sendData = {0};
	sendData.contType = 2;
	return write(sockfd,&sendData,sizeof(struct post_content));
}


int connectServer(char *ipaddr,int port)
{
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
	inet_pton(AF_INET,ipaddr,&addr.sin_addr.s_addr);
	if(connect(sockfd,(struct sockaddr*)&addr,sizeof(addr))<0)
    {
        perror("connect error");
       	return -1;
    }
	return sockfd;
}


void *pthread_showCont(void *arg)
{
	struct post_content sendData = {0};
	struct get_content *getParams = (struct get_content *)arg;
	printf("func Addr 	%p\n",getParams->resultFun);
	while(1)
	{
		read(getParams->socket_fd,&sendData,sizeof(struct post_content));
		printf("rebuf 	%s\n",sendData.buff);
		getParams->resultFun(sendData.buff,getParams->cloudResult);
		sleep(1);
		memset(&sendData,0,sizeof(struct post_content));
	}
}

void create_readFunc(int sockfd,struct get_content *getParams,void (*funcResult)(char*,char **),char**resOut)
{
	getParams->socket_fd = sockfd;
	getParams->resultFun = funcResult;
	getParams->cloudResult = resOut;
	pthread_t thread;
    pthread_attr_t  attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    pthread_create(&thread,&attr,pthread_showCont,(void*)getParams);
}

#if 0
int main()
{
#if 1
	VoiceHandle voiceHandle;	
	voiceInit(&voiceHandle);
	HuwenVADHandle	vadHandle;
	huwenVadInit(&vadHandle,16000,0.5,0.4f,2,20);
	float scores[4];	
	char *buffer = (char *)calloc(2048,12);
	float *aec = (float *)calloc(2048,4);
	float *input = (float *)calloc(1024 * 12,4);
	short *output = (short *)calloc(1024,2);
	int doadir[4];	
#endif

	int fd = connectServer("192.168.1.116",8088);
	FILE * fp = fopen("mj.pcm","rb");
//	FILE * fp1 = fopen("helo.pcm","wb");
   	int ret = 0;
	char buff[2048] = "";
	writeRecStart(fd);
	struct get_content getParams = {0};
	create_readFunc(fd,&getParams,dealResult);
	struct post_content sendData = {0};
	int vadNumber = -1;
	char * pBigBuff = (char*)calloc(2048,12);
	while(1)
	{
//		ret = fread(sendData.buff,1,2048,fp);
#if 1

		memset(pBigBuff,0,12*2048);
		ret = fread(pBigBuff,12,2048,fp);


		micShortToFloat((short *)pBigBuff,input,aec,9,1024);
		voiceProcess(voiceHandle,input,output,aec,doadir);
		vadNumber = huwenVadFeedData(&vadHandle,(char *)output,2048,scores);
		if(vadNumber ==1)
#endif
		{
			sendData.contType = 1;
			sendData.contLength= ret;
			memcpy(sendData.buff,(char *)output,2048);
			write(fd,&sendData,sizeof(struct post_content));
			memset(&sendData,0,sizeof(struct post_content));
		}
		if(ret < 1)
		{	
			writeRecStop(fd);
			sleep(2);
			writeRecStart(fd);
			fclose(fp);
			fp = fopen("mj.pcm","rb");
			while(1);
//			continue;
			break;
		}

	}
}
#endif
