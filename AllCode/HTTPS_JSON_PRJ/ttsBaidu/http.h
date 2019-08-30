#ifndef _TTS_H_
#define _TTS_H_
#define PORT 80
#define BUFSIZE 1024
#define URL "tts.baidu.com"
#define HTTP_GET "GET http://tts.baidu.com/text2audio?cuid=48-5A-B6-47-0A-BB&lan=zh&ctp=1&pdt=90&tex="
#define HTTP_HOST " HTTP/1.1\r\nHost: tts.baidu.com\r\n"
#define HTTP_CONTENT "Connection: close\r\n"
#define HTTP_ACCEPT "Accept: */*\r\n"
#define HTTP_END "\r\n"
extern void getSocket(char *text,char **buffer,int *bufferSize);
#endif
