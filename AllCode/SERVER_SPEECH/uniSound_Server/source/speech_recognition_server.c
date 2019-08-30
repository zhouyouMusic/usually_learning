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
	int frame_size = 0,nbBytes = 0;
	opus_int16 out[MAX_FRAME_SIZE*CHANNELS];
	unsigned char pcm_bytes[MAX_FRAME_SIZE*CHANNELS*2];
	
	int clientFd = (int)arg;
	int retVal = 0,recvedCount = 0,bufLength = 0;
	pthread_mutex_t MUTEX_UniSd;
	pthread_mutex_init(&MUTEX_UniSd,NULL);
	
	USC_HANDLE handleUSC;
	struct recog_params recogParam = {0};
	strcpy(recogParam.appKey,USC_ASR_SDK_APP_KEY);
	strcpy(recogParam.secretKey,USC_ASR_SDK_SECRET_KEY);
	strcpy(recogParam.domain,RECOGNITION_FIELD_GENERAL);
	strcpy(recogParam.audioFormat,AUDIO_FORMAT_PCM_16K);
	recogParam.IsUseNlu = 0;
	retVal = init_speech_recognition(&handleUSC,&recogParam,&MUTEX_UniSd);	
	if(retVal < 0)
	{
		pthread_mutex_destroy(&MUTEX_UniSd);
		return;
	}
	struct post_content recvData = {0};	
	
	char uniSdOut[3836] = "";
	char recvedBuff[3836+800] = "";
	char restBuff[800] = "";
	int restLength = 0;
	int i = 0;
	char frameData[3836*2] = "";
	char frameRest[3836] = "";
	int frameCount = 0;
	int flagStarted = 0;

	char outSpeech[3836] = "";
	struct post_content recvCont = {0};
	while(1)
	{
		memset(&recvData,0,sizeof(struct post_content));
		retVal = recv(clientFd,&recvData,sizeof(struct post_content),0);
		if(retVal < 1)
			break;
		memcpy(frameData+frameCount,&recvData,retVal);
		frameCount += retVal;
		if(frameCount < 3836)
		{
			continue;
		}else{
			memset(&recvCont,0,3836);
			memcpy(&recvCont,frameData,3836);
			memset(frameRest,0,3836);
			memcpy(frameRest,frameData+3836,frameCount-3836);
			memset(frameData,0,3836 *2);
			memcpy(frameData,frameRest,frameCount-3836);
			frameCount = frameCount-3836;
			switch(recvCont.contType)
			{
				case 1:
						start_speech_recognition(handleUSC,&MUTEX_UniSd);
						flagStarted = 1;
						break;

				case 2:
						if(flagStarted)
						{
							 pthread_mutex_lock(&MUTEX_UniSd);
							 frame_size = opus_decode(decoder,recvCont.buff,recvCont.contLength, out, MAX_FRAME_SIZE, 0);
							 for(i=0;i<CHANNELS*frame_size;i++)
    						 {
         						pcm_bytes[2*i]=out[i]&0xFF;
         						pcm_bytes[2*i+1]=(out[i]>>8)&0xFF;
      						 }
							 pthread_mutex_unlock(&MUTEX_UniSd);
							automatic_speech_recognition(handleUSC,pcm_bytes,frame_size*CHANNELS*sizeof(short)
																			,uniSdOut,&MUTEX_UniSd);
				
						}
						break;

				case 3:
						if(flagStarted)
						{
							stop_speech_recognition(handleUSC,uniSdOut,&MUTEX_UniSd);
							flagStarted = 0;
							recvCont.contType = 3;
						}
						break;
				default:
						recvCont.contType = 3;
						break;
			}
			if(recvCont.contType == 3)
				break;
		}
		

	}
	if(flagStarted)
		usc_stop_recognizer(handleUSC);
	destroy_speech_recognition(handleUSC,&MUTEX_UniSd);
	if(strlen(uniSdOut)==0)
	{
		strcpy(uniSdOut,"NoVoiceInput");
	}
	retVal = send(clientFd,uniSdOut,strlen(uniSdOut)+1,0);
	close(clientFd);
	BASE_INFO_LOG(InfoLogHandle,"%s","end clinet\n");
	pthread_mutex_destroy(&MUTEX_UniSd);
}


void build_server()
{
	InfoLogHandle = logInitHandle(FILE_NAME,FILE_PATH,MAX_SIZE,KEEP_NUM);
	ErrLogHandle = logInitHandle(ERROR_FILE,FILE_PATH,MAX_SIZE,KEEP_NUM);
	
	pthread_mutex_init(&MUTEX_CLIENT,NULL);
	int err = 0;
	decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &err);
		
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
			BASE_INFO_LOG(InfoLogHandle,"%s","*********************clinet connected ******\n");
		pthread_mutex_lock(&MUTEX_CLIENT);
		    pthread_t thread;
            pthread_attr_t  attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
            pthread_create(&thread,&attr,speech_request,(void*)client_fd);
		pthread_mutex_unlock(&MUTEX_CLIENT);
        }
    }
    return;
}
int main()
{
	build_server();
}

