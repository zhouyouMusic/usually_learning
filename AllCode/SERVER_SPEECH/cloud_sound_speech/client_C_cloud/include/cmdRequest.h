#ifndef _CMD_REQUEST_H_
#define _CMD_REQUEST_H_


void  *cmdControlThread(void * argv);

void cmdPlayThread(char *quesCmd,char **mp3Data,int *mp3Size,struct decode_frame*framInfo);

int dataFilter(char **dataStr,char **dataStr1);


int isCharIn(char *str1,const char *str2,int cmpLen);

int socketCreate(char * ipAddress,int port);

void MP3BoradCast(char *sAddr,struct decode_frame*framInfo);

void dataBoradCast(char *httpName,char * GET_HOST,char * GET_HTTP,char *text,char **buffer,int *bufferSize,struct decode_frame*framInfo);

void getPostBuf(char **buff,char*quesCmd);


void robotCmdResponse(char *quesCmd,void (*playfunc)(struct play_params *),void (*pushBuff)(char**,int*,int));


#endif

