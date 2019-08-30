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
#include <sys/time.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <errno.h>
#include <iconv.h>
#include "md5.h"
#include "cJSON.h"
#include "madAlsa.h"
#include "jsonAPI.h"
//	http://tts.baidu.com/text2audio?lan=zh&ie=UTF-8&spd=2&text=
//	http://192.168.1.102:8088/platform/test/test?query="+question+"&type=other

char * UTF8toANSI(char *from){
    char *inbuf=from;
    size_t inlen = strlen(inbuf);
    size_t outlen = inlen *4;
    char *outbuf = (char *)malloc(inlen * 4 );
   	memset(outbuf,0,inlen*4);
    char *in = inbuf;
    char *out = outbuf;
    iconv_t cd=iconv_open("UTF-8","GBK");
    iconv(cd,&in,&inlen,&out,&outlen);
    iconv_close(cd);
    return outbuf;
}



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
			postStr = postStr + 1;
	}
	return backStr;
}

char* subJsonStr(char *jsonStr)
{
	int endCount = 1,length = 0,i = 0;
	char *jsonBufStr = (char *)calloc(50,BASE_BUF);
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


void getPostBuf(char **transBuf,char *body,char **ip)
{
	struct hostent *ttsHost;
	ttsHost = gethostbyname("robot-service.centaurstech.com");	
	char * ipAddr = inet_ntoa(*(struct in_addr*)ttsHost->h_addr_list[0]);
	printf("IP	-	%s\n",ipAddr);
	*ip = ipAddr;
	char * pPostBuf = *transBuf;
	sprintf(pPostBuf,"%s%s%s%s%s%s%s%s%d%s%s%s%s%s",
			"POST ","/api/chat"," HTTP/1.1\n",
			"HOST: ",ipAddr,":443\n",
			"Content-Type: application/x-www-form-urlencoded\n",
			"Content-Length: ",
			(int)strlen(body),
			"\r\n",
			"\r\n",
			body,
			"\r\n",
			"\r\n"
			);
	*transBuf = pPostBuf;
	printf("%s\n",pPostBuf);
}

void sslPostHttps(char *postBuf,int sockfd,char* ipAddr,char **respon,int *reSize)
{
	*respon = NULL;
	char buffer[1024] = "";
	char lineStr[2048] = "";
	int retVal = 0,isfind = 0,index = 0,i = 0,postLen = 0,sendLen = 0,count = 0;
	SSL_library_init();
	SSL_load_error_strings();
	SSL_CTX *ctx = SSL_CTX_new(SSLv23_client_method());
	if (ctx == NULL)
	{
		close(sockfd);
		return;
	} 
	SSL * ssl = SSL_new(ctx);
	if (ssl == NULL)
	{
		close(sockfd);
		return;
	}
	retVal = SSL_set_fd(ssl, sockfd);
	if (retVal == 0)
	{
		close(sockfd);
		return;
	}
	RAND_poll();
	while (RAND_status() == 0)
	{
		unsigned short rand_ret = rand() % 65536;
		RAND_seed(&rand_ret, sizeof(rand_ret));
	}
	retVal = SSL_connect(ssl);
	if (retVal != 1)
	{
		close(sockfd);
		return;
	}
	postLen = strlen(postBuf);
	while(sendLen < postLen)
	{
		retVal = SSL_write(ssl,postBuf+sendLen,postLen-sendLen);
		if(retVal == -1)
		{
			close(sockfd);
			return;
		}
		sendLen += retVal;
	}
	while ((retVal = SSL_read(ssl,buffer,1)) == 1) 
	{
		if (i < 4) 
		{
			if (*respon == NULL) 
			{
				lineStr[index++] = buffer[0];
				if (buffer[0] == ':' && isfind == 0) 
				{
					lineStr[index - 1] = '\0';
					if (memcmp(lineStr,"Content-Length",strlen("Content-Length")) == 0) 
					{
						isfind = 1;
						index = 0;
					}
				}
			}
			if (buffer[0] == '\r' || buffer[0] == '\n') 
			{
				if (isfind == 1 && *respon == NULL) 
				{
					lineStr[index] = '\0';
					*reSize = atoi(lineStr);
					*respon = (char *)malloc(*reSize);
				}
				index = 0;
				i++;
			} 
			else 
			{
				i = 0;
			}
		} 
		else 
		{
			
			(*respon)[index++] = buffer[0];
			if(buffer[0]=='{')
				count++;
			if(buffer[0]=='}')
				count--;
			if(count==0)
				break;
		}
	}
	close(sockfd);
	printf("%s\n",*respon);

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
	free(ttsBuf);
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


int isCharIn(char *str1,const char *str2,int cmpLen)
{
	int length = 0;
	while(*str1!='\0')
	{
		if(*str1==*str2)
		{	
			if(!(strncmp(str1,str2,cmpLen)))
			{
				return 0;
			}
				
		}
		str1++;
		length++;
		if(length==1000)
			return -1;
	}
	return -1;
}

int isCharEnd(char * buff)
{
	while(*buff!='\0')
	{
		if((*buff=='\n')&&(*(buff+1)=='0'))
		{
			return 0;
		}
		buff++;
	}
	return -1;
}


void jsonDataDealing(char * jsonBuf)
{
	int size = 0,i = 0;
	char * MP3DataBuf = NULL;
	char * MP3MusicBuf = NULL;
	int musicSize = 0;
	int mp3Size = 0;
	char *jsonSubBuf = subJsonStr(jsonBuf);
	cJSON * cjsRoot = cJSON_Parse(jsonSubBuf);
	char * cRoot = cJSON_Print(cjsRoot);
	printf("%s\n",cRoot);

	cJSON * cjsCont = cJSON_GetObjectItem(cjsRoot,"msg");
	cJSON * cjsData = cJSON_GetObjectItem(cjsRoot,"data");
	char * strData = cJSON_Print(cjsData);
	char * strCont = cJSON_Print(cjsCont);
	dataBoradCast(HTTP_BAIDU,BAIDU_HOST,BAIDU_HTTP,strCont,&MP3DataBuf,&mp3Size);
	boradMp3(mp3Size,MP3DataBuf,16000,1);
	if(!isCharIn(strData,"src",strlen("src")))
	{
		cJSON * srcAddr = cJSON_GetObjectItem(cjsData,"src");
		char * src = cJSON_Print(srcAddr);
		printf("%s\n",src);
	}
	#if 0
	if(!isCharIn(strCont,MUSIC_INFO,strlen("resultMusic")))
	{
		cJSON * cjsCmdResult = cJSON_GetObjectItem(cjsCont,"resultMusic");
		cJSON * subCont = cJSON_GetObjectItem(cjsCmdResult,"content");
		cJSON * subResult = cJSON_GetObjectItem(subCont,"result");
		cJSON * musicNode = NULL;
    	size = cJSON_GetArraySize(subResult);
		for(i=0;i<size;i++)
		{
			musicNode = cJSON_GetArrayItem(subResult,i);
			if(musicNode->type == cJSON_Object)
			{
				cJSON * songName = cJSON_GetObjectItem(musicNode,"songName");
				cJSON * singerName = cJSON_GetObjectItem(musicNode,"singerName");
				cJSON * songUrl = cJSON_GetObjectItem(musicNode,"songUrl");
				char * sgUrl = cJSON_Print(songUrl);
				char * sgName = cJSON_Print(songName);
				char * sgrName = cJSON_Print(singerName);
				printf("%s\n",sgName);
				printf("%s\n",sgrName);
				MP3BoradCast(sgUrl,&MP3MusicBuf,&musicSize);
				boradMp3(musicSize,MP3MusicBuf,44100,2);
				free(sgUrl);
				free(sgName);
				free(sgrName);
				free(MP3MusicBuf);
				sgUrl = NULL;
				sgName = NULL;
				sgrName = NULL;
				MP3MusicBuf = NULL;
				break;
			}
		}
	}
	else
	{
		cJSON * cjsCmdType = cJSON_GetObjectItem(cjsCont,"cmdType");
		cJSON * cjsCmdData = cJSON_GetObjectItem(cjsCont,"cmdData");
		char * cmdType = cJSON_Print(cjsCmdType);
		if(!strncmp(cmdType,"\"chat\"",6))
		{
			char * cmdData = cJSON_Print(cjsCmdData);
			char * ttsStr = createTtsStr(cmdData);
			dataBoradCast(HTTP_BAIDU,BAIDU_HOST,BAIDU_HTTP,ttsStr,&MP3DataBuf,&mp3Size);
			boradMp3(mp3Size,MP3DataBuf,16000,1);
			free(ttsStr);
			free(MP3DataBuf);
			MP3DataBuf = NULL;
			cmdData = NULL;
			ttsStr = NULL;
		}
		if(!strncmp(cmdType,"\"media\"",7))
		{
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
						cJSON * songsAddr = cJSON_GetObjectItem(tNode,"songSourceAddr");
						char * sourceAddress = cJSON_Print(songsAddr);
						printf("sourceAddress :	%s\n",sourceAddress);
						MP3BoradCast(sourceAddress,&MP3MusicBuf,&musicSize);
						boradMp3(musicSize,MP3MusicBuf,22050,1);
						free(sourceAddress);
						free(MP3MusicBuf);
						sourceAddress = NULL;
						MP3MusicBuf = NULL;
						break;
        			}
     			}	
			}
			free(cmdType);
			cmdType = NULL;
		}
	}
	free(jsonSubBuf);
	cJSON_Delete(cjsRoot);
	jsonSubBuf = NULL;
	#endif
}

void dataFilter(char **dataStr,char **dataStr1)
{
	int flag = 0;
	char * ptrMov = *dataStr;
	char * ptrBase = ptrMov;
	char * ptrBaseKeep = ptrMov;
	char * ptr1Mov = *dataStr1;
	char * ptr1Base = ptr1Mov;
	
	while(*ptrMov!='\0')
	{
		if((*ptrMov=='{')&&(strncmp(ptrMov,"{\"code\":\"200\"",strlen("{\"code\":\"200\""))==0))	/***		除去包头 		***/
			break;
		ptrMov++;
	}
	while(*(ptrMov+4)!='\0')
	{
		if(((*ptrMov=='\\')&&(*(ptrMov+1)=='\"'))\
		||((*ptrMov=='\"')&&(*(ptrMov+1)=='{'))\
		||((*ptrMov=='\"')&&(*(ptrMov+1)=='}')&&(*(ptrMov+2)=='}')\
		&&(*(ptrMov+3)!='}')&&(*(ptrMov-1)=='}')&&(*(ptrMov-2)=='}')))		/***1.	去除	\"前面的 \   只留下"  ***/
		{																	/***2.	去除	"{	前的 "		***/
			ptrMov = ptrMov + 1;											/***3.	去除    "}}	前面的 ", 条件4为了确保和2 中的 " 是一对""  ***/
		}																	
		else
		{
			if((*ptrMov=='f')&&(*(ptrMov+1)=='s')&&(*(ptrMov+2)=='.')&&(*(ptrMov+3)=='w'))
			{
				if(!strncmp(ptrMov,"fs.w.kugou.com",strlen("fs.w.kugou.com")))				/***	判断是否酷狗，用于去歌词	***/
				{
					flag = 1;
				}	
			}
			*ptr1Mov++ = *ptrMov++;
		}
	}
	ptrMov = ptr1Base;	
	if(flag == 1)
	{
		memset(ptrBase,0,50*BASE_BUF);
		while(*ptrMov!='\0')																/***	除去歌词	***/
		{
			if((*(ptrMov+1)=='l')&&(*(ptrMov+2)=='y'))
			{
				if(!strncmp(ptrMov+1,"lyrics\":\"",strlen("lyrics\":\"")))
				{
					while(*(ptrMov+2)!='\0')
					{
						if((*ptrMov=='\"')&&(*(ptrMov+1)==','))
							break;
						ptrMov++;
					}
					ptrMov += 2;														/*** 	除去歌词结尾的" 和 ，	 ***/
				}
			}
			if((*ptrMov=='\n')&&(*(ptrMov+1)!='0'))								/***		除去每次带出的长度数值		***/
			{
				do{
					ptrMov++;				
				}while(*ptrMov!='\n');
			}
			*ptrBase++ = *ptrMov++;
		}
		ptrMov = ptrBaseKeep;
	}
	*dataStr = ptrMov;
}


void uCharToChar(unsigned char * verify,char **ques)
{
	int i = 0,j = 0;
	int flag = 1;
	char * pQues = *ques;
	char * pStart = pQues;
	unsigned char uch;
	char buff[3] = "";
	for(i=0;i<16;i++)
	{
		uch = *(verify + i);
		sprintf(buff,"%02x",uch);
		for(j = 0;j < 2;j++)	
			*(pQues+j) = buff[j];
		pQues += 2;
	}
	*ques = pStart;
}



void bodyHttpData(char **very,char **desBody,char *msg)
{
//		  "appkey=%s&timestamp=%s&uid=%s&verify=%s&msg=%s&nickname=%s",
	char *verStr = *very;
	char *bodyStr = *desBody;
	int i = 0;
	unsigned char decrypt[16] = {0};
	char verifyStr[100] = "";
	char* appkey = "huwen-demo";
	char * uid = "huwen";
	char * appsecret = "d5825cad568f15e2518e85d0dfe1f523";
	char * nickname = "张三李四王老五";
	struct timeval tme;
	gettimeofday(&tme,NULL);
	long int milliSec = tme.tv_sec*1000 + tme.tv_usec/1000;
	sprintf(verifyStr,"%s%s%ld",appsecret,uid,milliSec);
	MD5_CTX md5;
	MD5Init(&md5);
	MD5Update(&md5,verifyStr,strlen(verifyStr));
	MD5Final(&md5,decrypt);
	uCharToChar(decrypt,&verStr);	
	sprintf(bodyStr,"%s%s%s%ld%s%s%s%s%s%s%s%s",
			"appkey=",
			appkey,
			"&timestamp=",
			milliSec,
			"&uid=",
			uid,
			"&verify=",
			verStr,
			"&msg=",
			msg,
			"&nickname=",
			nickname
			);
	*desBody = bodyStr;
}


int main()
{
	char rsp_buf[2048] = {0};
	int sockfd = 0,retVal = 0;
	char * resPonse;
	int retSize;
	char * verifyStr = (char *)calloc(33,1);
	char * bodyStr = (char *)calloc(1,800);
	char * postStr = (char *)calloc(1,1024);
	char * jsonStr = (char *)calloc(1,1024);
	char * ipAddr = (char *)calloc(1,20);
	char sendMes[1024] = "";
	printf(">>>>>>>	");
	scanf("%s",sendMes);
	bodyHttpData(&verifyStr,&bodyStr,sendMes);
//	https_post_request_wait_rsp(NULL,"robot-service.centaurstech.com",443,bodyStr,rsp_buf,2024);
	getPostBuf(&postStr,bodyStr,&ipAddr);
	sockfd = socketCreate(ipAddr,443);
	sslPostHttps(postStr,sockfd,ipAddr,&resPonse,&retSize);	
	jsonDataDealing(resPonse);
}

