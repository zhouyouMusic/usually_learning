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
#include<unistd.h>
#include<time.h>
#include <sys/time.h>
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
//	FILE * fp = fopen("test.pcm","rb");
	char recvedBuff[2800] = "";
	char restBuff[800] = "";
	int restLength = 0;
	int i = 0;
	char frameData[4112] = "";
	char frameRest[2056] = "";
	int frameCount = 0;
	int flagStarted = 0;

	char outSpeech[2048] = "";
	struct post_content recvCont = {0};
	while(1)
	{
		memset(&recvData,0,sizeof(struct post_content));
		retVal = recv(clientFd,&recvData,sizeof(struct post_content),0);
		if(retVal < 1)
			break;
		memcpy(frameData+frameCount,&recvData,retVal);
		frameCount += retVal;
		if(frameCount < 2056)
		{
			continue;
		}else{
			memset(&recvCont,0,2056);
			memcpy(&recvCont,frameData,2056);
			memset(frameRest,0,2056);
			memcpy(frameRest,frameData+2056,frameCount-2056);
			memset(frameData,0,4112);
			memcpy(frameData,frameRest,frameCount-2056);
			frameCount = frameCount-2056;
			switch(recvCont.contType)
			{
				case 1:
						start_speech_recognition(handleUSC);
						flagStarted = 1;
						break;

				case 2:
						bufLength = 2048;
						recvedCount = bufLength + restLength;
						memset(recvedBuff,0,2800);
						memcpy(recvedBuff,restBuff,restLength);
						memcpy(recvedBuff+restLength,recvCont.buff,bufLength);
						i = 0;
						while((recvedCount / 640) > 0)
						{
							automatic_speech_recognition(handleUSC,recvedBuff+(i*640),640,clientFd);
							i++;
							recvedCount -= 640;
						}
						restLength = recvedCount;
						memcpy(restBuff,recvedBuff+(i*640),restLength);
						break;

				case 3:
						if(restLength > 0)
							automatic_speech_recognition(handleUSC,restBuff,restLength,clientFd);
						stop_speech_recognition(handleUSC,clientFd);
						flagStarted = 0;
						break;
				default:
						break;
			}
			if(recvCont.contType == 3)
				break;
		}
		

	}
	if(flagStarted)
		usc_stop_recognizer(handleUSC);
	destroy_speech_recognition(handleUSC);
	close(clientFd);
	BASE_INFO_LOG(InfoLogHandle,"%s","end clinet\n");
	
}


void build_server()
{
	
	InfoLogHandle = logInitHandle(FILE_NAME,FILE_PATH,MAX_SIZE,KEEP_NUM);
	ErrLogHandle = logInitHandle(ERROR_FILE,FILE_PATH,MAX_SIZE,KEEP_NUM);

	
	pthread_mutex_init(&MUTEX_CLOUD_SPEECH,NULL);
	
    int server_fd = 0,client_fd = 0;
    struct sockaddr_in address;
    memset(&address,0,sizeof(address));
    server_fd = socket(AF_INET,1,0);
	
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
#if 1
    while(1)
    {
        client_fd = accept(server_fd,NULL,NULL);
		usleep(10000);
        if(client_fd < 0)
        {
            printf("退出\n");
			BASE_ERROR_LOG(ErrLogHandle,"%s","client fd is  connect error\n");
            return;
        }else{
#endif
			BASE_INFO_LOG(InfoLogHandle,"%s","*********************clinet connected ******\n");
		    pthread_t thread;
            pthread_attr_t  attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
            pthread_create(&thread,&attr,speech_request,(void*)client_fd);
        }
    }
    return;
}
int main()
{
	build_server();
}

