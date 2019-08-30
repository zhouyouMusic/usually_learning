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
#include "libusc.h"
#include "cloud_sounds_recognition.h"

void *speech_request(void *arg)
{
	USC_HANDLE handleUSC;
	struct recog_params recogParam = {0};
	strcpy(recogParam.appKey,USC_ASR_SDK_APP_KEY);
	strcpy(recogParam.secretKey,USC_ASR_SDK_SECRET_KEY);
	strcpy(recogParam.domain,RECOGNITION_FIELD_GENERAL);
	strcpy(recogParam.audioFormat,AUDIO_FORMAT_PCM_16K);
	recogParam.IsUseNlu = 0;
	init_speech_recognition(&handleUSC,&recogParam);	

	
	struct post_content recvData = {0};	
	int clientFd = (int)arg;
	int retVal = 0;
	int recvedCount = 0;
	int bufLength = 0;
	FILE * fp1 = fopen("send.pcm","wb");
	FILE * fp = fopen("test.pcm","rb");
//	char recvedBuff[2800] = "";
	char  *recvedBuff = malloc(2800);
	char restBuff[800] = "";
	char feedBuff[2560] = "";
	char cloudBuf[640] = "";
	int restLength = 0;
	int i = 0;
	printf("~begin~~~~~~~~~~~~~~~~~~~~~~~&&&&&&&&&&&&&	\n");
	start_speech_recognition(handleUSC);
	while(1)
	{
		
//		memset(&recvData,0,sizeof(struct post_content));

//		retVal = recv(clientFd,&recvData,sizeof(struct post_content),0);
		retVal = fread(recvData.buff,1,1152,fp);
//		bufLength = retVal - 8;
		bufLength = retVal;

		printf("%d\n",retVal);
//		recvData.contType = 1;
		
		if(retVal < 1)
			break;
//		recvData.contLength = 1152;

#if 0		
		if(recvData.contType == 0)
		{
			start_speech_recognition(handleUSC);
			continue;
		}
		if(recvData.contType == 2)
		{
			stop_speech_recognition(handleUSC,clientFd);
			continue;
		}
#endif
//		if(recvData.contLength!=0)
		{
			recvedCount = bufLength + restLength;
			memset(recvedBuff,0,2800);
			memcpy(recvedBuff,restBuff,restLength);
			memcpy(recvedBuff+restLength,recvData.buff,bufLength);
			while((recvedCount / 640) > 0)
			{
				memcpy(cloudBuf,recvedBuff+(i*640),640);
				fwrite(cloudBuf,1,640,fp1);
	//			automatic_speech_recognition(handleUSC,recvedBuff+(i*640),640,clientFd);
				i++;
				recvedCount -= 640;
			}
			restLength = recvedCount;
			memcpy(restBuff,recvedBuff+(i*640),restLength);
			
			
#if 0		
			memcpy(recvedBuff+recvedCount,recvData.buff,retVal-8);
			recvedCount += retVal -8;
			if(recvedCount+restLength > 2047)
			{
				memcpy(feedBuff,restBuff,restLength);
				memcpy(feedBuff+restLength,recvedBuff,2048-restLength);
				memset(restBuff,0,2048);
				memcpy(restBuff,recvedBuff+2048-restLength,recvedCount+restLength-2048);
				memset(recvedBuff,0,4096);
				restLength = recvedCount-2048+restLength;
				recvedCount = 0;
				automatic_speech_recognition(handleUSC,feedBuff,2048,clientFd);
				memset(feedBuff,0,2048);

			}
#endif

		}
	}
	stop_speech_recognition(handleUSC,clientFd);
	printf("over~~~~~~~~~~~~~~~~~~~~~~~~&&&&&&&&&&&&&	\n");
	fflush(stdout);
	destroy_speech_recognition(handleUSC);
	/*	
		switch(recvData.contType)
		{
			case 0:
					printf("staring~~~~~~~~~~~~~~recognition	~~~~~~~~~~~~~~~~~~~\n");
					start_speech_recognition(handleUSC);
					break;
			case 1:
					printf("auto~~~~~~~~~~~~~~~~~~~~~~recognition %d\n",retVal);
//					fwrite(recvData.buff,recvData.contLength,1,fp);
					automatic_speech_recognition(handleUSC,recvData.buff,recvData.contLength,clientFd);
					break;
					
			case 2:
					printf("stop~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~recognition\n");
					stop_speech_recognition(handleUSC,clientFd);
					break;

			case 3:
//					destroy_speech_recognition(handleUSC);
					break;
					
			default:
					break;
		}		
	*/
	
//	printf("end client	@@@@@@@@@@@@@@@@\n");
	
}


void build_server()
{

    int server_fd = 0,client_fd = 0;
    struct sockaddr_in address;
    memset(&address,0,sizeof(address));
    server_fd = socket(AF_INET,1,0);
#if 0

    address.sin_family = AF_INET;
    address.sin_port = htons(8998);
    address.sin_addr.s_addr = INADDR_ANY;
    if(bind(server_fd,(struct sockaddr*)&address,sizeof(address))<0)
    {
        perror("service bind error");
        exit(1);
    }
    if(listen(server_fd,1024) < 0)
    {
        perror("service listen error");
        exit(1);
    }
    while(1)
    {
        printf("正在链接.......\n");
        client_fd = accept(server_fd,NULL,NULL);
		usleep(100);
        if(client_fd < 0)
        {
            printf("退出\n");
            return;
        }else{
#endif
			pthread_t thread;
            pthread_attr_t  attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
            pthread_create(&thread,&attr,speech_request,(void*)client_fd);
 //       }
  //  }
 //   pthread_attr_destroy(&attr);
 	while(1);
    return ;
}
int main()
{
	build_server();
}

