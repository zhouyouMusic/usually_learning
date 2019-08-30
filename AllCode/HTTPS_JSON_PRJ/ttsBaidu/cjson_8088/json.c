#include <stdio.h>
#include <fcntl.h>
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
#include "cJSON.h"
#include "madAlsa.h"
#include "jsonAPI.h"
//	http://tts.baidu.com/text2audio?lan=zh&ie=UTF-8&spd=2&text=
//	http://192.168.1.102:8088/platform/test/test?query="+question+"&type=other


NUM_CODING getNumCoding(char *num)
{
	NUM_CODING numCd = {0};
	numCd.chA = *num;
	numCd.chB = *(num + 1);
	numCd.chC = *(num + 2);
	return numCd;
}

void codingNum(char chr,char *newBuf)
{
	NUM_CODING numCd = {0};
	switch(chr)
	{
		case '0':
				numCd = getNumCoding("零");
				break;
		case '1':
				numCd = getNumCoding("一");
				break;
		case '2':
				numCd = getNumCoding("二");
				break;
		case '3':
				numCd = getNumCoding("三");
				break;
		case '4':
				numCd = getNumCoding("四");
				break;
		case '5':
				numCd = getNumCoding("五");
				break;
		case '6':
				numCd = getNumCoding("六");
				break;
		case '7':
				numCd = getNumCoding("七");
				break;
		case '8':
				numCd = getNumCoding("八");
				break;
		case '9':
				numCd = getNumCoding("九");
				break;
		case 'b':
				numCd = getNumCoding("百");
				break;
		case 's':
				numCd = getNumCoding("十");
				break;
	}	
	*newBuf = numCd.chA;
	*(newBuf+1) = numCd.chB;
	*(newBuf+2) = numCd.chC;
}


char *createTtsStr(char *postStr)
{
	printf("createStr 	%s	\n",postStr);
	unsigned char chr1 = 0,chr2 = 0,chr3 = 0;
	int i = 0,flag = 0;
	char *ttsStr = (char *)calloc(1,1024);
	char *backStr = ttsStr;
	while(*postStr!='\0')
	{	
		chr1 = *postStr;
		chr2 = *(postStr + 1);
		chr3 = *(postStr + 2);
		if(chr1>=0xb0 && chr2>=0x80 && chr3 >= 0x80) 
		{
			if((chr1==0xef) ||(chr1==0xe3 && chr2 == 0x80))
			{
				if(flag == 0)
				{
					*(ttsStr+i) = ',';			//将中文符号转换成英文 ","号,
					i++;				//将中文所占的三个字节替换成一个字节
					flag = 1;
				}		
			}
			else
			{
				*(ttsStr + i) = chr1;					//如果是汉字则保留三字节编码
				*(ttsStr + i + 1) = chr2;
				*(ttsStr + i + 2) = chr3;
				flag = 0;
				i += 3;
			}
			postStr += 3;
		}	
		else if(isalpha(chr1))				//如果是字母则保留
		{
			*(ttsStr + i) = chr1;
			i++;
			postStr = postStr + 1;
			flag = 0;
		}
		else if(chr1 >= 48 && chr1 <= 57)		//如果是数字，则转换成中文的数字
		{
			codingNum(chr1,ttsStr+i);
			i += 3;
			if((chr2 >= 48 && chr2 <= 57)&& (chr1!=48))			//判断是否是至少两位数
			{
				if(chr3 >= 48 && chr3 <= 57)			//判断是否是至少三位数,不支持三位数以上播报
				{
					codingNum('b',ttsStr+i);
					i += 3;
					if((chr3 != 48)&&(chr2 != 48))								//当第三位不是0，那么这个三位数要全部读出来
					{
						codingNum(chr2,ttsStr+i);
						i += 3;
						codingNum('s',ttsStr+i);
						i += 3;					
						codingNum(chr3,ttsStr+i);
						i += 3;
					}
					else if((chr3 != 48)&&(chr2 == 48)) 
					{
						codingNum(chr2,ttsStr+i);
						i += 3;
						codingNum(chr3,ttsStr+i);
						i += 3;
					}	
					else if((chr3==48)&&(chr2!=48))				//当第三位是0,则比如三百二十，最后一个0可不读
					{
						codingNum(chr2,ttsStr+i);
						i += 3;
						codingNum('s',ttsStr+i);
						i += 3;					
					}	
					postStr += 3;
				}
				else
				{
					codingNum('s',ttsStr+i);
					i += 3;
					if(chr2 != 48)							//两位数时，如果第二位是0，也可以不读出来，只读比如二十
					{
						codingNum(chr2,ttsStr+i);
						i += 3;
					}
					postStr += 2;
				}
			}
			else
				postStr = postStr + 1;
			flag = 0;
		}	
		else
			postStr = postStr +1;
	}
	return backStr;
}

char* subJsonStr(char *jsonStr)
{
	int endCount = 1,length = 0,i = 0;
	char *jsonBufStr = (char *)calloc(2,QUES_BUF);
	if(NULL == jsonBufStr)
	{
		perror("jsonBufStr  calloc  error\n");
		return NULL;
	}
	char *jsonKeepBuf = jsonBufStr;
	while(*jsonStr!='{')
	{
		*jsonStr++;					//移除随机出现的以1C1之类开头的形式
	}
	while((*jsonBufStr++=*jsonStr++)!='\0')
	{
		if(*jsonStr=='{')
			endCount++;
		if(*jsonStr=='}')
			endCount--;
		if(endCount==0)
		{
			*jsonBufStr++=*jsonStr++;
			break;
		}
	}
	*jsonBufStr = '\0';
	return jsonKeepBuf;
}


char * getPostBuf()
{
	char * POST_HOST = "HOST: 210.22.153.226:8088\n";
	char * POST_HTTP = "POST /platform/test/test?query=";
	char * POST_TYPE = "Content-Type: application/x-www-form-urlencoded\n";
	char * TYPE_OTHER = "&type=other";
	char * quesBuf = (char *)calloc(sizeof(char),QUES_BUF);
	char * headBuf = (char *)calloc(sizeof(char),HEAD_BUF);
	char * postBuf = (char *)calloc(sizeof(char),POST_BUF);
	if((NULL==quesBuf)||(NULL==headBuf)||(NULL==postBuf))
	{
		perror("quesBuf headBuf postBuf  calloc  error\n");
		return NULL;
	}
	
	int retVal = 0;
	printf("Please input your question ,end with the key Enter\n");
	printf(">>>>>># ");
	scanf("%s",quesBuf);
	if(strlen(quesBuf)>= QUES_BUF)
	{
		perror("your question is too long ,input shorter please");
		return NULL;
	}
	sprintf(headBuf,"%s%s%s%s",POST_HTTP,quesBuf,TYPE_OTHER," HTTP/1.1\n");
	sprintf(postBuf,"%s%s%s%s",
			headBuf,
			POST_HOST,
			POST_TYPE,
			"\r\n"
		);
	printf("%s\n",postBuf);
	free(quesBuf);
	free(headBuf);
	quesBuf = NULL;
	headBuf = NULL;
	return postBuf;
}

char * getTtsBuf(char *transBuf,char * GET_HOST,char * GET_HTTP)
{
	
	int txtSize = 1024;
	char * headBuf = (char *)calloc(sizeof(char),txtSize);
	char * ttsBuf = (char *)calloc(sizeof(char),txtSize);
	if((NULL==headBuf)||(NULL==ttsBuf))
	{
		perror("headBuf ttsBuf calloc Error");
		return NULL;
	}
	sprintf(headBuf,"%s%s%s",GET_HTTP,transBuf," HTTP/1.1\r\n");
	sprintf(ttsBuf,"%s%s%s%s%s",
	 		headBuf,
	 		GET_HOST,
	 		"Connection: close\r\n",
	 		"Accept: */*\r\n",
	 		"\r\n"
			 );
	 free(headBuf);
	 return ttsBuf;
}


void dataBoradCast(char *httpName,char * GET_HOST,char * GET_HTTP,char *text,char **buffer,int *bufferSize) 
{
	int ttsSockfd = 0,length = -1,i = 0,index = 0,isfind = 0,retVal = 0;
	struct hostent *ttsHost;
	ttsHost = gethostbyname(httpName);	
	//	printf("address type: %s \n",(ttsHost->h_addrtype==AF_INET)?"AF_INET":"AF_INET6");
	char * ipAddr = inet_ntoa(*(struct in_addr*)ttsHost->h_addr_list[0]);
	printf("%s\n",ipAddr);
	ttsSockfd = socketCreate(ipAddr,80);
	char * ttsBuf = getTtsBuf(text,GET_HOST,GET_HTTP);
	retVal = write(ttsSockfd,ttsBuf,strlen(ttsBuf));
	char url[1024];
	memset(url,'\0',1024);
	char line[2049];
	*buffer = NULL;
	while ((length = read(ttsSockfd,url,1)) > 0) {
		if (i < 4) {
			if (*buffer == NULL) {
				line[index++] = url[0];
				if (url[0] == ':' && isfind == 0) {
					line[index - 1] = '\0';
					if (memcmp(line,"Content-Length",strlen("Content-Length")) == 0) {
						isfind = 1;
						index = 0;
					}
				}
			}
			if (url[0] == '\r' || url[0] == '\n') {
				if (isfind == 1 && *buffer == NULL) {
					line[index] = '\0';
					*bufferSize = atoi(line);
					*buffer = (char *)malloc(*bufferSize);
				}
				index = 0;
				i++;
			} else {
				i = 0;
			}
		} else {
			(*buffer)[index++] = url[0];
		}
	}
	close(ttsSockfd);
}

void MP3BoradCast(char *sAddr,char **buffer,int *bufferSize)
{
	char buff_GET[200] = "";
	char buff_HOST[50] = "";
	char buff_HTTP[50] = "";
	int i = 0;
	sAddr = sAddr + 1;
	*(sAddr + strlen(sAddr)-1) = '\0';
	sprintf(buff_GET,"%s%s","GET ",sAddr);
	printf("%s\n",sAddr);
	sAddr = sAddr + strlen("http://");
	printf("%s\n",sAddr);
	while((*sAddr != '\n') && (*sAddr != '\0'))
	{
		if(*sAddr == '/')
			break;
		buff_HTTP[i++] = *sAddr++;
	}
	sprintf(buff_HOST,"%s%s%s","HOST: ",buff_HTTP,"\r\n");
	dataBoradCast(buff_HTTP,buff_HOST,buff_GET,"",buffer,bufferSize);
}

int socketCreate(char * ipAddress,int port)
{
	int uSocket = 0;
	uSocket = socket(AF_INET,SOCK_STREAM,0);
	if(uSocket < 0)
	{
		perror(" failed to create socket");
		return -1;
	}
	struct sockaddr_in sockAddr;
	memset(&sockAddr,0,sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(port);
	if(inet_pton(AF_INET,ipAddress,&sockAddr.sin_addr) <= 0)
	{
		perror( "failed to convert 	IPADDR to internet IP");
		return -1;
	}	
	if(connect(uSocket,(struct sockaddr *)&sockAddr,sizeof(struct sockaddr_in)) < 0)
	{
		perror("failed to connect to the server");
		return -1;
	}
	return uSocket;

}

void jsonDataDealing(char * jsonBuf)
{
	printf("jsonBuf = %s\n",jsonBuf);
	char *jsonSubBuf = subJsonStr(jsonBuf);
	cJSON * cjsRoot = cJSON_Parse(jsonSubBuf);
	cJSON * cjsCont = cJSON_GetObjectItem(cjsRoot,"content");
	cJSON * cjsCmdType = cJSON_GetObjectItem(cjsCont,"cmdType");
	cJSON * cjsCmdData = cJSON_GetObjectItem(cjsCont,"cmdData");
	char * cmdType = cJSON_Print(cjsCmdType);
//	char * joke = cJSON_Print(cjsRoot);
//	printf("%s\n",joke);
	char * MP3DataBuf = NULL;
	char * MP3MusicBuf = NULL;
	int musicSize = 0;
	int mp3Size = 0;
	if(!strncmp(cmdType,"\"chat\"",6))
	{
		char * cmdData = cJSON_Print(cjsCmdData);
		char * ttsStr = createTtsStr(cmdData);
		dataBoradCast(HTTP_BAIDU,BAIDU_HOST,BAIDU_HTTP,ttsStr,&MP3DataBuf,&mp3Size);
		boradMp3(mp3Size,MP3DataBuf,16000,1);
//		dataBoradCast(ttsStr);
		free(cmdData);
		free(ttsStr);
		free(MP3DataBuf);
		MP3DataBuf = NULL;
		cmdData = NULL;
		ttsStr = NULL;
	}
	if(!strncmp(cmdType,"\"media\"",7))
	{
		cJSON * cjsCommandType = cJSON_GetObjectItem(cjsCmdData,"commandType");
		char * commandType = cJSON_Print(cjsCommandType);
		printf("%s		cmdtype\n",commandType);
		cJSON * musicData = cJSON_GetObjectItem(cjsCmdData,"data");
		cJSON * musicInfo = cJSON_GetObjectItem(musicData,"musicInfo");
		if(musicInfo->type == cJSON_Array)
		{
			cJSON *tNode = NULL;
    		int size = cJSON_GetArraySize(musicInfo);
     		int i;
    		for(i=0;i<size;i++)
     		{
         		tNode = cJSON_GetArrayItem(musicInfo,i);
         		if(tNode->type == cJSON_Object)
         		{
         			cJSON * musicArtist = cJSON_GetObjectItem(tNode,"artist");
					cJSON * songsAddr = cJSON_GetObjectItem(tNode,"songSourceAddr");
					cJSON * songsName = cJSON_GetObjectItem(tNode,"songName");
					char * artist = cJSON_Print(musicArtist);
					char * sourceAddress = cJSON_Print(songsAddr);
					char * songName = cJSON_Print(songsName);
					printf("sourceAddress :	%s\n",sourceAddress);
					printf("artist :	%s\n",artist);
					printf("songsName :	%s\n",songName);
					MP3BoradCast(sourceAddress,&MP3MusicBuf,&musicSize);
					if(!strncmp(commandType,"\"CommandMusic\"",strlen("\"CommandMusic\"")))
						boradMp3(musicSize,MP3MusicBuf,44100,2);
					if(!strncmp(commandType,"\"CommandJoke\"",strlen("\"CommandJoke\"")))
						boradMp3(musicSize,MP3MusicBuf,16000,1);
					free(artist);
					free(sourceAddress);
					free(songName);
					free(MP3MusicBuf);
					free(commandType);
					artist = NULL;
					sourceAddress = NULL;
					songName = NULL;
					break;
        		}
     		}	
		}
	}
	free(cmdType);
	free(jsonSubBuf);
	cJSON_Delete(cjsRoot);
	cmdType = NULL;
	jsonSubBuf = NULL;

}

int main()
{
	int PORTS = 8088;
	char * IPADDR = "210.22.153.226";
	char * CODE_OK = "\"code\":\"200\"";
	int retVal = 0,uSocket = 0;
	char * postBuf = getPostBuf();
	uSocket = socketCreate(IPADDR,PORTS);
	char * jsonBuf = (char *)calloc(2,JSON_BUF);
	char * jsonBufDeal = jsonBuf;
	if(NULL==jsonBuf)
	{
		perror("jsonBuf calloc error\n");
		return -1;
	}
	retVal = write(uSocket,postBuf,strlen(postBuf));
	if(retVal <= 0)
	{
		perror("send question error");
		return -1;
	}
	while(1)
	{
		retVal = read(uSocket,jsonBuf,JSON_BUF);
		if(retVal < 1)
			break;
		jsonBuf += retVal;
	}
	jsonBuf = jsonBufDeal;
	if(!strstr(jsonBuf,CODE_OK))
	{
		while(1)
		{
			retVal = read(uSocket,jsonBuf,1*JSON_BUF);
			if(retVal < 1)
				break;
			jsonBuf += retVal;
		}
		printf("read twice\n");
	}
	jsonBuf = jsonBufDeal;
	jsonDataDealing(jsonBuf);
	free(postBuf);
	free(jsonBuf);
	postBuf = NULL;
	jsonBuf = NULL;
}




