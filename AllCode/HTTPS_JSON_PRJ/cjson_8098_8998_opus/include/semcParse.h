#ifndef _ALSA_PLAY_H_
#define _ALSA_PLAY_H_
#include <semaphore.h>
#include "alsaInterface.h"
#include "cmdReqInterface.h" 
#include "cJSON.h"
#include "speex/speex_resampler.h"


#define  MEDIA_URL   "media_url_"
#define  TTS_TEXT    "tts_text_"

sem_t Sem_SmdParse;


#define WITHOUT_PALYING  0
#define MEDIA_PLAYING    1
#define TTS_PLAYING      2
#define CONTROL_CMD		 3



typedef struct 
{
	char *ipAddr;
	int  port;
	char * buff_HTTP;
	char * req_Content;
	char * ctlCmdJSON;
	int  smdParType;
	char *  quesCmdBuff;						/************quesCmd ********/
	char *  postHttpReq;						/************store  request cmd	 to get 8098 content *****/	
	char *  jsonBaseBuff;						/************for json Dealing process *******/
	char *  jsonTempBuff;						/************for json Dealing process 	too*******/	
}SmdParsingBuffer;

int MP3BoradCast(char *sAddr,SemcParseHandle smcPreHand);
extern SemcParseHandle semcParseHandInit();
extern void robotCmdResponse(char *quesCmd,SemcParseHandle semcPrHand,char **smdRlt);


#endif

