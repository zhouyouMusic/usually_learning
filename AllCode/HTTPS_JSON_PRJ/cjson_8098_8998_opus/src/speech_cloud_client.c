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
#include<netinet/tcp.h>
#include<unistd.h>
#include<netdb.h>
#include "logInterface.h"
#include "speechBaseModule.h"


void pourArryData(char *arryPre,char *arryAft,int *ptrInt)
{
	int arryTemp = *ptrInt;
	int arryDeep  = arryTemp;
	
	while(arryTemp)
	{
		memcpy(arryAft+((arryDeep-arryTemp)*sizeof(struct post_content)),
			arryPre+((arryTemp-1)*sizeof(struct post_content)),
			sizeof(struct post_content));
		arryTemp--;
	}	
}

void dataPushQueqe(char *arryA,int *ptrInt,int *pStp,struct post_content *inCnt) 
{
	if(arryA == NULL)
	{
		memcpy(arryA,inCnt,sizeof(struct post_content));
		(*ptrInt)++;
	}else{
		if((*ptrInt) > SQUEQUE_MAX_DEEP)
		{
			BASE_ERROR_LOG(ErrLogHandle,"%s","can`t input, ptrInt Error ,it`s  over size\n");
			return;
		}else if((*ptrInt) == SQUEQUE_MAX_DEEP){
			if(*pStp != 0)
			{
				memset(arryA+(sizeof(struct post_content)*((*pStp) -1)),0,
						(sizeof(struct post_content)*(SQUEQUE_MAX_DEEP+1-(*pStp))));
				*ptrInt = *pStp - 1;
			}else{
				BASE_ERROR_LOG(ErrLogHandle,"%s","can`t input,pStp is zero, but buffer is full\n");
				return;
			}
		}else{
			memcpy(arryA+(sizeof(struct post_content)*(*ptrInt)),inCnt,
					sizeof(struct post_content));
			(*ptrInt)++;
		}
	}
	if(inCnt->contType == 3)		/***********ptrInt point to head place of the squeqe ***************/
	{
		*pStp == *ptrInt;		/***********pStp point to place of the last message that contain stop flag*******/
	}
}


int dataPullQueqe(char *arryA,char *arryB,int *ptrInt,int *pStp,struct post_content *outCnt) 
{
	if((arryA == NULL) || *(ptrInt)==0)
	{
		return -1;
	}else{
		memcpy(outCnt,arryA,sizeof(struct post_content));
		memset(arryB,0,sizeof(struct post_content)*SQUEQUE_MAX_DEEP);
		--(*ptrInt);
		if(*pStp > 0)
			--(*pStp);
		pourArryData(arryA+sizeof(struct post_content),arryB,ptrInt);
		memset(arryA,0,sizeof(struct post_content)*SQUEQUE_MAX_DEEP);
		pourArryData(arryB,arryA,ptrInt);
		return 0;
	}
}


int socket_timeout(int fd,unsigned int wait_seconds)
{
	int ret=0;           
	if(wait_seconds>0)  
	{
		fd_set sockfd_fdset;     
    	struct timeval timeout;  
     
     	FD_ZERO(&sockfd_fdset);
     	FD_SET(fd,&sockfd_fdset);
     
     	timeout.tv_sec = wait_seconds;
     	timeout.tv_usec = 0;
     	do
     	{
     	   ret=select(fd+1,&sockfd_fdset,NULL,NULL,&timeout);   
     	}while(ret<0 && errno == EINTR);  	
    	 if(ret==0) 
    	 {
      	   ret=-1;
       	   errno= ETIMEDOUT;
     	}
     	else if(ret==1)
       	ret= 0;
 	}
	return ret;
}


void  *wrtRcdData(void * argv)
{
	SPEECH_REG_HANDLE * uniSdHandle  = argv;
	struct post_content postData = {0};
	int retV = 0,clientFd = 0,sendLen = 0,FlagStarted = 0;
	char speechOut[2048] = "";
	while(1)
	{
		memset(&postData,0,sizeof(struct post_content));
		pthread_mutex_lock(&(uniSdHandle->mutexSndRThd));
			retV = dataPullQueqe(uniSdHandle->arryA,uniSdHandle->arryB,
								&(uniSdHandle->ptrInt),&(uniSdHandle->pStp),&postData);
		pthread_mutex_unlock(&(uniSdHandle->mutexSndRThd));
		if(retV == 0)
		{
			BASE_INFO_LOG(InfoLogHandle,"%s%d%s","postData contType- ",postData.contType,"\n");
			
			switch(postData.contType)
			{

				case 1:

						clientFd = connectServer(uniSdHandle->serverAddr,uniSdHandle->serPort,
																	uniSdHandle->func_result);

						if(clientFd <= 0)
							break;
						FlagStarted = 1;

						struct timeval timeoutSnd = {3,0};
						struct timeval timeoutRcv = {5,0};
			            retV = setsockopt(clientFd,SOL_SOCKET,SO_SNDTIMEO,
											(char *)&timeoutSnd,sizeof(struct timeval));
						if(retV < 0)
							BASE_ERROR_LOG(ErrLogHandle,"%s","setsockopt for timeout error \n");
						retV = setsockopt(clientFd,SOL_SOCKET,SO_RCVTIMEO,
											(char *)&timeoutRcv,sizeof(struct timeval));
						if(retV < 0)
							BASE_ERROR_LOG(ErrLogHandle,"%s","setsockopt for timeout error \n");

						sendLen = send(clientFd,&postData,sizeof(struct post_content),0);
						if(sendLen < 0)
							BASE_ERROR_LOG(ErrLogHandle,"%s","write start signal to server error \n");
						break;

				case 2:
						if(FlagStarted)
						{
							if(clientFd <= 0)
								break;
							sendLen = send(clientFd,&postData,sizeof(struct post_content),0);	
							if(sendLen < 0)
							{	
								BASE_ERROR_LOG(ErrLogHandle,"%s","write data to server error \n");
							}
						}
						break;
				case 3:
						if(FlagStarted)
						{	
							if(clientFd <= 0)
								break;
		
							sendLen = send(clientFd,&postData,sizeof(struct post_content),0);
							if(sendLen < 0)
							{	
								BASE_ERROR_LOG(ErrLogHandle,"%s","write stop signal to server error \n");
							}
							memset(speechOut,0,2048);
							retV = recv(clientFd,speechOut,2048,0);
							if(retV < 0)
							{
								BASE_ERROR_LOG(ErrLogHandle,"%s","recv speechOut Error\n");
								strcpy(speechOut,"RequestOverTime");
							}
							if(!(uniSdHandle->ifCancel))
							{ 
								uniSdHandle->func_result(speechOut);
							}else{
								pthread_mutex_lock(&(uniSdHandle->mutexSndRThd));
									uniSdHandle->ifCancel = 0;
									memset(uniSdHandle->arryA,0,sizeof(struct post_content)*SQUEQUE_MAX_DEEP);
								pthread_mutex_unlock(&(uniSdHandle->mutexSndRThd));
							}
							close(clientFd);
							clientFd = 0;
							FlagStarted = 0;		
						}
						BASE_INFO_LOG(InfoLogHandle,"%s%s","here over speech once ","\n");
						break;
			}
		}else{
			usleep(100);
		}
	}	
}

extern HwSoundHandle hwSoundHandleInit(char *ipAr,int port,void (*func_res)(char*))
{
	SPEECH_REG_HANDLE * uniSdHandle = (SPEECH_REG_HANDLE *)malloc(sizeof(SPEECH_REG_HANDLE));
	uniSdHandle->arryA = (char*)malloc(sizeof(struct post_content) *SQUEQUE_MAX_DEEP);
	uniSdHandle->arryB = (char*)malloc(sizeof(struct post_content) *SQUEQUE_MAX_DEEP);
	memset(uniSdHandle->arryA,0,sizeof(struct post_content) *SQUEQUE_MAX_DEEP);
	memset(uniSdHandle->arryB,0,sizeof(struct post_content) *SQUEQUE_MAX_DEEP);
	uniSdHandle->ptrInt = 0;
	uniSdHandle->pStp = 0;
	memset(&(uniSdHandle->pstCont),0,sizeof(struct post_content));
	pthread_mutex_init(&(uniSdHandle->mutexUniSd),NULL);
	
	strcpy(uniSdHandle->serverAddr,ipAr);
	uniSdHandle->serPort = port;
	uniSdHandle->func_result = func_res;
	pthread_mutex_init(&(uniSdHandle->mutexSndRThd),NULL);

	uniSdHandle->ifCancel = 0;

	int err;
	uniSdHandle->opusCnt.encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, APPLICATION, &err);
	err = opus_encoder_ctl(uniSdHandle->opusCnt.encoder, OPUS_SET_BITRATE(BITRATE));
	 
	pthread_t wrtPthread;
	pthread_attr_t	wrtControl;
	pthread_attr_init(&wrtControl);
	pthread_attr_setdetachstate(&wrtControl,PTHREAD_CREATE_DETACHED);
	if(pthread_create(&wrtPthread,&wrtControl,wrtRcdData,(void *)uniSdHandle)!=0)
				printf("\npthread_create error\n"); 		
		
	return (HwSoundHandle)uniSdHandle;
}

extern void hwSoundRecgStart(HwSoundHandle usdHandle)
{
	SPEECH_REG_HANDLE * uniSdHandle = (SPEECH_REG_HANDLE *)usdHandle;
	pthread_mutex_lock(&(uniSdHandle->mutexUniSd));
		memset(&(uniSdHandle->pstCont),0,sizeof(struct post_content));
		uniSdHandle->pstCont.contType = 1;
		uniSdHandle->pstCont.contLength = 0;
		memset(uniSdHandle->opusCnt.opusFrameBuff,0,8192);
		uniSdHandle->opusCnt.opusBuffLen = 0;
		
		dataPushQueqe(uniSdHandle->arryA,&(uniSdHandle->ptrInt),
						&(uniSdHandle->pStp),&(uniSdHandle->pstCont));
	pthread_mutex_unlock(&(uniSdHandle->mutexUniSd));
}


extern void hwSoundRecgStop(HwSoundHandle usdHandle)
{
	SPEECH_REG_HANDLE * uniSdHandle = (SPEECH_REG_HANDLE *)usdHandle;
	pthread_mutex_lock(&(uniSdHandle->mutexUniSd));
	if(uniSdHandle->opusCnt.opusBuffLen != 0)
	{
#if 1
		memset(&(uniSdHandle->pstCont),0,sizeof(struct post_content));
		uniSdHandle->pstCont.contType = 2;
		uniSdHandle->pstCont.contLength = FRAME_SIZE;
		memset(uniSdHandle->opusCnt.opusFrameBuff+uniSdHandle->opusCnt.opusBuffLen,0,
										FRAME_BYTES - uniSdHandle->opusCnt.opusBuffLen);
		int i = 0;
		memset(uniSdHandle->opusCnt.opusInput,0,FRAME_SIZE*CHANNELS);
		for(i=0;i<CHANNELS*FRAME_SIZE;i++)
		{
   			uniSdHandle->opusCnt.opusInput[i] = \
				uniSdHandle->opusCnt.opusFrameBuff[2*i+1]<<8|uniSdHandle->opusCnt.opusFrameBuff[2*i];
		}	
		uniSdHandle->pstCont.contLength = opus_encode(uniSdHandle->opusCnt.encoder,uniSdHandle->opusCnt.opusInput,\
									FRAME_SIZE, uniSdHandle->opusCnt.cbitsBuf, MAX_PACKET_SIZE);
		dataPushQueqe(uniSdHandle->arryA,&(uniSdHandle->ptrInt),
							&(uniSdHandle->pStp),&(uniSdHandle->pstCont));
#endif
	}
		
	memset(&(uniSdHandle->pstCont),0,sizeof(struct post_content));
	uniSdHandle->pstCont.contType = 3;
	uniSdHandle->pstCont.contLength = 0;
	dataPushQueqe(uniSdHandle->arryA,&(uniSdHandle->ptrInt),
					&(uniSdHandle->pStp),&(uniSdHandle->pstCont));
	BASE_INFO_LOG(InfoLogHandle,"%s%s","insert to squeqe over^^^","\n");	
	pthread_mutex_unlock(&(uniSdHandle->mutexUniSd));
}

extern void hwSoundFeedData(HwSoundHandle usdHandle,unsigned char *feedBuff,int feedLen)
{
	SPEECH_REG_HANDLE * uniSdHandle = (SPEECH_REG_HANDLE *)usdHandle;
	pthread_mutex_lock(&(uniSdHandle->mutexUniSd));
		memset(&(uniSdHandle->pstCont),0,sizeof(struct post_content));
		uniSdHandle->pstCont.contType = 2;
		memcpy(uniSdHandle->opusCnt.opusFrameBuff+uniSdHandle->opusCnt.opusBuffLen,feedBuff,feedLen);
		if((uniSdHandle->opusCnt.opusBuffLen + feedLen) < FRAME_BYTES)
		{
			uniSdHandle->opusCnt.opusBuffLen += feedLen;
		}else{
			int i = 0;
			memset(uniSdHandle->opusCnt.opusInput,0,FRAME_SIZE*CHANNELS);
			for(i=0;i<CHANNELS*FRAME_SIZE;i++)
			{
       			uniSdHandle->opusCnt.opusInput[i] = \
					uniSdHandle->opusCnt.opusFrameBuff[2*i+1]<<8|uniSdHandle->opusCnt.opusFrameBuff[2*i];
			}	
			uniSdHandle->pstCont.contLength = opus_encode(uniSdHandle->opusCnt.encoder,uniSdHandle->opusCnt.opusInput,\
										FRAME_SIZE, uniSdHandle->opusCnt.cbitsBuf, MAX_PACKET_SIZE);
			memcpy(uniSdHandle->pstCont.buff,uniSdHandle->opusCnt.cbitsBuf,uniSdHandle->pstCont.contLength);
			uniSdHandle->opusCnt.opusBuffLen = uniSdHandle->opusCnt.opusBuffLen + feedLen - FRAME_BYTES;
			
			memcpy(uniSdHandle->opusCnt.opusFrameBuff,uniSdHandle->opusCnt.opusFrameBuff + FRAME_BYTES,
																		uniSdHandle->opusCnt.opusBuffLen);
			
			dataPushQueqe(uniSdHandle->arryA,&(uniSdHandle->ptrInt),
									&(uniSdHandle->pStp),&(uniSdHandle->pstCont));
		}
		if(uniSdHandle->opusCnt.opusBuffLen > FRAME_BYTES)
			BASE_ERROR_LOG(InfoLogHandle,"%s%d%s","leftOpusLen :  ",uniSdHandle->opusCnt.opusBuffLen,"\n");	
		
	pthread_mutex_unlock(&(uniSdHandle->mutexUniSd));
}

extern void hwSoundDestroy(HwSoundHandle usdHandle)
{
	SPEECH_REG_HANDLE * uniSdHandle = (SPEECH_REG_HANDLE *)usdHandle;
	pthread_mutex_destroy(&(uniSdHandle->mutexUniSd));	
	pthread_mutex_destroy(&(uniSdHandle->mutexSndRThd));	
	free(uniSdHandle);
}

extern void hwSoundCancel(HwSoundHandle usdHandle)
{
	SPEECH_REG_HANDLE * uniSdHandle = (SPEECH_REG_HANDLE *)usdHandle;
	pthread_mutex_lock(&(uniSdHandle->mutexUniSd));
		uniSdHandle->ifCancel = 1;
	pthread_mutex_unlock(&(uniSdHandle->mutexUniSd));
	hwSoundRecgStop(usdHandle);
}


int connectServer(char *ipaddr,int port,void (*func_result)(char*))
{
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
	inet_pton(AF_INET,ipaddr,&addr.sin_addr.s_addr);
	int conRet = 0;
	time_t curSeconds,preSeconds;
	preSeconds = time((time_t *)NULL);

	while(1)
	{
		conRet= connect(sockfd,(struct sockaddr*)&addr,sizeof(addr));

		if(conRet >= 0)
			break;
		usleep(100000);
		curSeconds = time((time_t *)NULL);
		if(curSeconds > (preSeconds + 4))
		{
			func_result("CLOUDSERVER_ERROR");
			preSeconds = time((time_t *)NULL);
			return -1;
		}

	}
	BASE_INFO_LOG(InfoLogHandle,"%s%s%s","Succeed Connect ",ipaddr,"\n");

	return sockfd;
}




