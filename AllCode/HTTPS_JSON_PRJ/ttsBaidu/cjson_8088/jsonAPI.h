#ifndef __JSON_API_H
#define __JSON_API_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct
{
	unsigned chA;
	unsigned chB;
	unsigned chC;
}NUM_CODING;

#define QUES_BUF 1024
#define POST_BUF 1200
#define HEAD_BUF 1200
#define JSON_BUF 2048
#define BAIDU_HOST "HOST: tts.baidu.com\r\n"
#define BAIDU_HTTP "GET http://tts.baidu.com/text2audio?cuid=48-5A-B6-47-0A-BB&lan=zh&ctp=1&pdt=90&tex="
#define HTTP_BAIDU "tts.baidu.com"


char* subJsonStr(char *jsonStr);
void dataBoradCast(char *httpName,char * GET_HOST,char * GET_HTTP,char *text,char **buffer,int *bufferSize) ;


char * getPostBuf();
char * getTtsBuf(char *transBuf,char * GET_HOST,char * GET_HTTP);
int socketCreate(char * ipAddress,int port);
void jsonDataDealing(char * jsonBuf);

NUM_CODING getNumCoding(char *num);
void codingNum(char chr,char *newBuf);





#endif
