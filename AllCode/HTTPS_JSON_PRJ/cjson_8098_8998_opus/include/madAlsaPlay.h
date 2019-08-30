#ifndef _MAD_ALSA_PLAY_H_
#define _MAD_ALSA_PLAY_H_

#include "alsaInterface.h"
#include "cJSON.h"
#include "speex/speex_resampler.h"
#include "logInterface.h"

typedef long long int madAlsaPlayHandle;
#define MAX_STREAM_BUFFER (50 * 1024)
#define PULL_ONCE_SIZE 1024	


int   SOCKET_FOR_CLOSE;
int   CONTROL_STREAM_FLAG;
int   THROW_FRAME_LEFT;


#define BASE_BUF 1024
#define BIG_BUF  1200

#define BAIDU_HOST "HOST: tts.baidu.com\r\n"
#define BAIDU_HTTP "GET http://tts.baidu.com/text2audio?cuid=48-5A-B6-47-0A-BB&lan=zh&ctp=1&pdt=90&vol=1&tex="
#define HTTP_BAIDU "tts.baidu.com"
 




#define PAUSE_RESUME_PAUSE    1
#define PAUSE_RESUME_RESUME   2
#define PAUSE_RESUME_STOP     3
#define DOWN_VOICE 	          4
#define UPPER_VOICE           5
#define CHANGE_SONGS          6

sem_t PTHREAD_CHANGE;
sem_t SEM_PLAY_READED;
sem_t SEM_PLAY_WRITED;
char commonBuff[1024 *8];


pthread_mutex_t MUTEX_DwStm;


struct play_params
{
	madAlsaPlayHandle madAlsaHandle; 
 	snd_mixer_t **		    handleMixer;
  	snd_mixer_elem_t **    handleElement;
	unsigned char *outPtr;
	int srLength;
	int preTTSFlag;
	int *pauseFlag;
	int samRate;
	int chnNum;
};

struct buffer {
  unsigned long       length;
  unsigned long       lengPre;
  void (*playMP3)(struct play_params *);
  madAlsaPlayHandle madAlsaHd; 
  snd_mixer_t *		    mixer;
  snd_mixer_elem_t *    element;
};


typedef struct 
{
	SpeexResamplerState *resampler16;
	SpeexResamplerState *resampler225;
	SpeexResamplerState *resampler48;
	int16_t  int16InBuf[4096];
	int16_t  *int16OutBuf;
	char     *outPlayBuf;
	AlsaHandle AlsaPlayHandle;
	char *	chPlayPre;
	char *	chPlayNow;
	char *	chPlayStart;
	char * stmArryA;
	char * stmArryB;
	int   IntDeep;
	pthread_mutex_t MUTEX_AlsaPlay;
	pthread_mutex_t MUTEX_Stm;
	pthread_mutex_t MUTEX_HttpDown;
	char *  httpMp3Buff;
}MadDecAlsaBuffer;


void playMusic(struct play_params* param);

int decPlayingThread(AlsaHandle alsaPlayHd);

void dataBoradCast(char *httpName,char * GET_HOST,char * GET_HTTP,char *text,madAlsaPlayHandle madAlsaHd);

void dataBoradPlay(char *httpName,char *postMp3Http,madAlsaPlayHandle madAlsaHd);


madAlsaPlayHandle  madDecAlsaPlayInit(char *alsaPlayDev);

#endif


