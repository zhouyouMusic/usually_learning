#ifndef _BASE_MODULE_H_
#define _BASE_MODULE_H_
#include "logInterface.h"

#define BASE_BUF 1024
#define BIG_BUF  1200

#define true 	1
#define false	0

#define PERROR(RC,STR)\
{\
	if(RC < 0)\
	{\
		BASE_ERROR_LOG(ErrLogHandle,"%s%s",STR,"\n");\
		exit(-1);\
	}\
}

int ifContainEnglish(char *quesCmd);

void getPostBuf(char **buff,char*quesCmd,char *ipAddr,int port);

int contentIsUtf8(char *str);

int socketCreate(char * ipAddress,int port);

int isCharIn(char *str1,const char *str2,int cmpLen);


#endif
