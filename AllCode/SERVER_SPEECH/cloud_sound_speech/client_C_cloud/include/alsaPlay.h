#ifndef _ALSA_PLAY_H_
#define _ALSA_PLAY_H_
#include "alsaInterface.h"


#define PAUSE_RESUME_PALY     1
#define PAUSE_RESUME_PAUSE    2
#define PAUSE_RESUME_RESUME   3
#define PAUSE_RESUME_STOP     4
#define DOWN_VOICE 	          5
#define UPPER_VOICE           6
#define CHANGE_SONGS          7
#define TTS_CMD_PALY		  8

 

char commonBuff[1024 *8];


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

	AlsaHandle *handlePlay;
// 	snd_mixer_t **		    handleMixer;
//  	snd_mixer_elem_t **    handleElement;
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
  unsigned char  *start;
  unsigned char  *pre;
  unsigned char  *playing;
  unsigned long       length;
  unsigned long       lengPre;
  int *SET_PCM_FLAG;
  int *PAUSE_FLAG;
  int *ENDING_FLAG;
  void (*playMP3)(struct play_params *);
  void (*gettingBuff)(char**,int*,int);

  AlsaHandle palyHandle;
//  snd_mixer_t *		    mixer;
//  snd_mixer_elem_t *    element;
};

struct decode_frame
{
	char * tts_buffer;
	int   tts_length;
	char * frame_data;
	int  frame_length;
	int *init_pcm_flag;
	int *control_flag;
	int *ending_flag;
	int *sockfdPre;
	int  *over_cmd_flag;
	void (*playfunc)(struct play_params *);
	void (*pushBuff)(char**,int*,int);
};


struct current_frame_over
{
	char  * quesCmd;
	int   * over_cmd_flag;
	void (*playfunc)(struct play_params *);
	void (*pushBuff)(char**,int*,int);
};
struct buffer_input
{
	struct buffer *buff;
	struct mad_decoder *decoder;
};

//int end_decoding_flag = 1;


void playMusic(struct play_params* param);

void fflushCommonBuffer(char **cData,int *cLength,int ffFlag);

int decode(struct decode_frame *framInfo,struct buffer *buffer,struct mad_decoder*decoder,int*initFlag);


#endif

