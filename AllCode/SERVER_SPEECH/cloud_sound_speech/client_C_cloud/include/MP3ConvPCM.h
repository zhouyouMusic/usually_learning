#ifndef _MP3_CONV_PCM_H_
#define _MP3_CONV_PCM_H_
#include "mad.h"
#include "alsaPlay.h"


#define BASE_BUF 1024
#define BIG_BUF  1200


#define BAIDU_HOST "HOST: tts.baidu.com\r\n"
#define BAIDU_HTTP "GET http://tts.baidu.com/text2audio?cuid=48-5A-B6-47-0A-BB&lan=zh&ctp=1&pdt=90&tex="
#define HTTP_BAIDU "tts.baidu.com"

#if 0

#define PERROR(RC,STR)\
{\
	if(RC < 0)\
	{\
		printf(STR"\n");\
		exit(1);\
	}\
}


struct play_params
{
	snd_pcm_t** handlePlay;
	snd_pcm_hw_params_t**	 handleParams;
 	snd_mixer_t **		    handleMixer;
  	snd_mixer_elem_t **    handleElement;
	unsigned char *outPtr;
	int srLength;
	int *pauseFlag;
	int *setPcmFlag;
	int samRate;
	int chnNum;
	
};

//snd_pcm_t*            handle;   
//snd_pcm_hw_params_t*  params;

struct buffer {
  unsigned char const *start;
  unsigned long       length;
  int *SET_PCM_FLAG;
  int *PAUSE_FLAG;
  void (*playMP3)(struct play_params *);
  snd_pcm_hw_params_t*	params;
  snd_pcm_t*            handle;  
  snd_mixer_t *		    mixer;
  snd_mixer_elem_t *    element;
};

struct decode_frame
{
	char * frame_data;
	int  frame_length;
	int *init_pcm_flag;
	int *control_flag;
	void (*playfunc)(struct play_params *);
};

#endif
//snd_mixer_t *		    mixer;
//snd_mixer_elem_t *    element;




char* subJsonStr(char *jsonStr);


void getPostBuf(char **buff,char*quesCmd);

char * getTtsBuf(char *transBuf,char * GET_HOST,char * GET_HTTP);
int socketCreate(char * ipAddress,int port);
void jsonDataDealing(char * jsonBuf,char **mp3Data,int *mp3Size,struct decode_frame*framInfo);

int dataFilter(char **dataStr,char **dataStr1);
void  *cmdControlThread(void * argv);





#endif 
