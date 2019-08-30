#include <stdio.h>
#include <fcntl.h>
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
#include <sys/ioctl.h>
#include <sys/time.h>
#include <alsa/asoundlib.h>
#include <linux/soundcard.h>


#include "mad.h"
#include "semcParse.h"
#include "madAlsaPlay.h"
#include "baseModule.h"

madAlsaPlayHandle madAlsaHandTemp;
SemcParseHandle  semcParseHandTemp;
pthread_mutex_t		MUTEX_Test;

snd_mixer_t * sndMix;
snd_mixer_elem_t * sndELEM;


struct newThreadPlay
{
	madAlsaPlayHandle madAHd;
	SemcParseHandle  semcPHd;
};


void linkPlayParse(char *parOut,SemcParseHandle semcPrHand)
{
//printf("%s	\n",parOut);
	int i = 0;
	char urlAddr[1000] = "";
	SmdParsingBuffer * smdParseHandle = (SmdParsingBuffer *)semcPrHand;
	memset(smdParseHandle->buff_HTTP,0,50);
	memset(smdParseHandle->req_Content,0,4096);
	smdParseHandle->smdParType = WITHOUT_PALYING;
	if(!isCharIn(parOut,MEDIA_URL,strlen(MEDIA_URL)))
	{
		while(parOut[i] != '\0')
		{
			if(parOut[i] == '\"')
			{
				strcpy(urlAddr,parOut+i);
				break;
			}
			i++;
		}
		urlAddr[strlen(urlAddr)] = '\0';
		if(MP3BoradCast(urlAddr,semcPrHand) < 0)
				return;
		smdParseHandle->smdParType = MEDIA_PLAYING;
	}else if(!isCharIn(parOut,TTS_TEXT,strlen(TTS_TEXT))){
		smdParseHandle->smdParType = TTS_PLAYING;
		while(parOut[i] != '\0')
		{
			if(parOut[i] == '\"')
			{
				strcpy(urlAddr,parOut+(i+1));
				break;
			}
			i++;
		}
		urlAddr[strlen(urlAddr)-1] = '\0';
		strcpy(smdParseHandle->req_Content,urlAddr);
	}else if(!isCharIn(parOut,"mediaPause",strlen("mediaPause"))){
		pthread_mutex_lock(&MUTEX_Test);
			CONTROL_STREAM_FLAG = PAUSE_RESUME_PAUSE;
		pthread_mutex_unlock(&MUTEX_Test);
	}else if(!isCharIn(parOut,"mediaResume",strlen("mediaResume"))){
		pthread_mutex_lock(&MUTEX_Test);
			CONTROL_STREAM_FLAG = PAUSE_RESUME_RESUME;
		pthread_mutex_unlock(&MUTEX_Test);
	}else if(!isCharIn(parOut,"mediaStop",strlen("mediaStop"))){
		pthread_mutex_lock(&MUTEX_Test);
			if(SOCKET_FOR_CLOSE > 0)
			{
				close(SOCKET_FOR_CLOSE);
					SOCKET_FOR_CLOSE = 0;
			}
		pthread_mutex_unlock(&MUTEX_Test);
	}else if((!isCharIn(parOut,"mediaNext",strlen("mediaNext")))||\
			(!isCharIn(parOut,"mediaPrevious",strlen("mediaPrevious")))){
		pthread_mutex_lock(&MUTEX_Test);
			CONTROL_STREAM_FLAG = CHANGE_SONGS;	
		pthread_mutex_unlock(&MUTEX_Test);
	}else if(!isCharIn(parOut,"homeControl",strlen("homeControl"))){
		smdParseHandle->smdParType = TTS_PLAYING;
		strcpy(smdParseHandle->req_Content,"当前未连接控制设备");
	}else if((!isCharIn(parOut,"CommandSystemVolume",strlen("CommandSystemVolume")))\
	 &&(!isCharIn(parOut,"3",strlen("3")))){
		int curVal = curVoiceValue(sndMix,sndELEM);
		changeVoiceValue(curVal+10,sndELEM);
		pthread_mutex_lock(&MUTEX_Test);
			CONTROL_STREAM_FLAG = PAUSE_RESUME_RESUME;
		pthread_mutex_unlock(&MUTEX_Test);
	}else if((!isCharIn(parOut,"CommandSystemVolume",strlen("CommandSystemVolume")))\
	 &&(!isCharIn(parOut,"1",strlen("1")))){
		int curVal = curVoiceValue(sndMix,sndELEM);
		changeVoiceValue(curVal-10,sndELEM);
		pthread_mutex_lock(&MUTEX_Test);
			CONTROL_STREAM_FLAG = PAUSE_RESUME_RESUME;
		pthread_mutex_unlock(&MUTEX_Test);
	}
}

void *streamPlayThread(void *arg)
{
	struct newThreadPlay * newThdPy =  arg;
	madAlsaPlayHandle madAPHd = newThdPy->madAHd;
	SemcParseHandle semcPHd = newThdPy->semcPHd;
	SmdParsingBuffer * smdParseHandle = (SmdParsingBuffer *)semcPHd;
	MadDecAlsaBuffer * madDecHandle = (MadDecAlsaBuffer *)madAPHd;
	if(smdParseHandle->smdParType == MEDIA_PLAYING)
	{
		dataBoradPlay(smdParseHandle->buff_HTTP,smdParseHandle->req_Content,madAPHd);
		return;
	}
	if(smdParseHandle->smdParType == TTS_PLAYING)
	{
		dataBoradCast(HTTP_BAIDU,BAIDU_HOST,BAIDU_HTTP,smdParseHandle->req_Content,madAPHd);
		return;
	}


}

void madAlsaPlaying(char **parOut,SemcParseHandle semcPrHand,madAlsaPlayHandle alsaPlayHd)
{	
	linkPlayParse(*parOut,semcPrHand);
	static struct newThreadPlay newThdPlay;
	newThdPlay.madAHd = alsaPlayHd;
	newThdPlay.semcPHd = semcPrHand;
	SmdParsingBuffer * smdParseHandle = (SmdParsingBuffer *)semcPrHand;
	MadDecAlsaBuffer * madDecHandle = (MadDecAlsaBuffer *)alsaPlayHd;
#if 1
	if(CONTROL_STREAM_FLAG == CHANGE_SONGS)
	{
		semcParseResponse("放歌",semcPrHand,parOut);
		linkPlayParse(*parOut,semcPrHand);
	}
#endif
	if((smdParseHandle->smdParType == MEDIA_PLAYING)||\
		(smdParseHandle->smdParType == TTS_PLAYING))
	{
		pthread_t StreamThread;
    	pthread_attr_t  streamControl;
    	pthread_attr_init(&streamControl);
    	pthread_attr_setdetachstate(&streamControl,PTHREAD_CREATE_DETACHED);	
		if(pthread_create(&StreamThread,&streamControl,streamPlayThread,(void*)&newThdPlay)!=0)
   			PERROR(-1,"\npthread_create error");	
	}

}
  
char * smdRlt;
void print_func(char * resBuf)
{
//	printf("%s\n",resBuf);
	semcParseResponse(resBuf,semcParseHandTemp,&smdRlt); 
	madAlsaPlaying(&smdRlt,semcParseHandTemp,madAlsaHandTemp);
}


int main()
{
	pthread_mutex_init(&MUTEX_Test,NULL);	
	CONTROL_STREAM_FLAG = 0;
	SOCKET_FOR_CLOSE = 0;
	THROW_FRAME_LEFT = 0;
	
	InfoLogHandle = logInitHandle(FILE_NAME,FILE_PATH,MAX_SIZE,KEEP_NUM);
	ErrLogHandle = logInitHandle(ERROR_FILE,FILE_PATH,MAX_SIZE,KEEP_NUM);
	
	madAlsaPlayHandle alsaPlayHand = madDecAlsaPlayInit("default");
	madAlsaHandTemp = alsaPlayHand;

	smdRlt = (char *)malloc(1024);
	semcParseHandTemp = semcParseHandInit();
	

	set_mixer(&sndMix,&sndELEM);
	
	BASE_INFO_LOG(InfoLogHandle,"%s","start from main func!!!!!!\n");
	rcdSpeechRecognition(print_func,2,8);

}

















