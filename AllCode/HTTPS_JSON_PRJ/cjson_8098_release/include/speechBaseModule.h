#ifndef _SPEECH_CLOUD_CLIENT_
#define _SPEECH_CLOUD_CLIENT_
#include <semaphore.h>
#include "cmdReqInterface.h"

#define SQUEQUE_MAX_DEEP 150

struct post_content
{
	int contType;
	int contLength;
	char buff[2048];
};


typedef struct 
{
	char * arryA;
	char * arryB;	
	int   ptrInt;
	int   pStp;
	struct post_content pstCont;
	pthread_mutex_t	mutexUniSd;
	char serverAddr[20];
	int serPort;
	void (*func_result)(char*);
	pthread_mutex_t	mutexSndRThd;
	int ifCancel;
}SPEECH_REG_HANDLE;

	
void pourArryData(char *arryPre,char *arryAft,int *ptrInt);

void dataPushQueqe(char *arryA,int *ptrInt,int *pStp,struct post_content *inCnt);

int dataPullQueqe(char *arryA,char *arryB,int *ptrInt,int *pStp,struct post_content *outCnt);

int connectServer(char *ipaddr,int port,void (*func_result)(char*));



#endif
