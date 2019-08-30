#ifndef CLOUD_SOUNDS_RECOGNITION_H
#define CLOUD_SOUNDS_RECOGNITION_H
#define _ZHOU_

#ifndef _ZHOU_
#define USC_ASR_SDK_APP_KEY "zxjfctw7izukawafj4fu53yzraojlndombm2uuyk"
#define USC_ASR_SDK_SECRET_KEY "59d67bcec12e9eb22ac1d0a23b800ff4"
#endif



#ifdef _ZHOU_
	#define USC_ASR_SDK_APP_KEY 	"vov26jpwyzg4ayxec3qoklmtslev2m4a36ka7tac"
	#define USC_ASR_SDK_SECRET_KEY	"b818fb13ead37df1ef050c6060629b54"
#endif 

/*********
vov26jpwyzg4ayxec3qoklmtslev2m4a36ka7tac
b818fb13ead37df1ef050c6060629b54

***/

#include "libusc.h"
#include "logInterface.h"
#include <pthread.h>

struct recog_params
{
	char  domain[50];
	char  audioFormat[50];
	int   IsUseNlu;
	char  NluBuff[50];
	char  appKey[100];
	char  secretKey[100];
};

struct post_content
{
	int contType;
	int contLength;
	char buff[2048];
};

pthread_mutex_t  MUTEX_CLIENT;


void init_speech_recognition(USC_HANDLE* handle,struct recog_params * recogParam,pthread_mutex_t *pMutex);

int start_speech_recognition(USC_HANDLE handle,pthread_mutex_t *pMutex);

int stop_speech_recognition(USC_HANDLE handle,char *uniOut,pthread_mutex_t *pMutex);

void destroy_speech_recognition(USC_HANDLE handle,pthread_mutex_t *pMutex);

void automatic_speech_recognition(USC_HANDLE handle,char *pcmBuff,int pcmLength,char *uniOut,pthread_mutex_t *pMutex);


#endif


