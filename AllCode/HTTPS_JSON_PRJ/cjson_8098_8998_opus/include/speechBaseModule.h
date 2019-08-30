#ifndef _SPEECH_CLOUD_CLIENT_
#define _SPEECH_CLOUD_CLIENT_
#include <semaphore.h>
#include "cmdReqInterface.h"
#include "opus/opus.h"


#define SQUEQUE_MAX_DEEP 150


#define FRAME_SIZE 960
#define SAMPLE_RATE 48000
#define CHANNELS 2
#define APPLICATION OPUS_APPLICATION_AUDIO
#define BITRATE 64000

#define FRAME_BYTES (FRAME_SIZE*(sizeof(short))*CHANNELS)

#define MAX_FRAME_SIZE (6*960)
#define MAX_PACKET_SIZE (3*1276)


//unsigned char leftOpusBuf[OPUS_BUF_SIZE];
//unsigned int  leftOpusLen;

struct post_content
{
	int contType;
	int contLength;
	unsigned char buff[MAX_PACKET_SIZE];
};

struct opus_content
{
	unsigned char opusFrameBuff[8192];
	unsigned int opusBuffLen;
	opus_int16 opusInput[FRAME_SIZE*CHANNELS];
	opus_int16 opusOutput[MAX_FRAME_SIZE*CHANNELS];
	unsigned char cbitsBuf[MAX_PACKET_SIZE];
	OpusEncoder *encoder;
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
	struct opus_content opusCnt;
}SPEECH_REG_HANDLE;

	
void pourArryData(char *arryPre,char *arryAft,int *ptrInt);

void dataPushQueqe(char *arryA,int *ptrInt,int *pStp,struct post_content *inCnt);

int dataPullQueqe(char *arryA,char *arryB,int *ptrInt,int *pStp,struct post_content *outCnt);

int connectServer(char *ipaddr,int port,void (*func_result)(char*));



#endif
