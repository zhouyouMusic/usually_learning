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
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/ioctl.h>
#include <alsa/asoundlib.h>
#include <linux/soundcard.h>
#include "MP3ConvPCM.h"
#include "cJSON.h"
#include "cmdRequest.h"     
#include "recordResult.h"


void downDataPlay(struct decode_frame *framInfo,struct buffer *buffer,struct mad_decoder*decoder,int*initFlag)
{
	decode(framInfo,buffer,decoder,initFlag);
}

void getPostBuf(char **buff,char*quesCmd)
{
	char * POST_HOST = "HOST: 210.22.153.226:8098\n";
	char * POST_HTTP = "POST /platform/test/test?query=";
	char * POST_TYPE = "Content-Type: application/x-www-form-urlencoded\n";
	char * TYPE_OTHER = "&type=other";
	char * quesBuf = (char *)calloc(sizeof(char),BASE_BUF);
	char * headBuf = (char *)calloc(sizeof(char),BIG_BUF);
	char * postBuf = *buff;
	if((NULL==quesBuf)||(NULL==headBuf)||(NULL==postBuf))
	{
		perror("quesBuf headBuf postBuf  calloc  error\n");
		return;
	}
	
	int retVal = 0;
	strncpy(quesBuf,quesCmd,strlen(quesCmd));
	
	if(strlen(quesBuf)>= BASE_BUF)
	{
		perror("your question is too long ,input shorter please");
		return;
	}
	sprintf(headBuf,"%s%s%s%s",POST_HTTP,quesBuf,TYPE_OTHER," HTTP/1.1\n");
	sprintf(postBuf,"%s%s%s%s",
			headBuf,
			POST_HOST,
			POST_TYPE,
			"\r\n"
		);
//	printf("%s\n",postBuf);
	free(quesBuf);
	free(headBuf);
	quesBuf = NULL;
	headBuf = NULL;
	*buff = postBuf;
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

void dataBoradPlay(char *httpName,char * GET_HOST,char * GET_HTTP,char *text,struct decode_frame*framInfo) 
{	
	if(*(framInfo->control_flag)== PAUSE_RESUME_PALY)
	{
		if(*(framInfo->sockfdPre) > 0)
			close(*(framInfo->sockfdPre));
		while(!(*(framInfo->ending_flag)))
			{
				printf("info ending play***************************888\n");
				sleep(1);
				fflush(stdout);
			}
		memset(commonBuff,0,1024*8);
		commonBuff[0] = 'N';
		printf("--------------------------------------------***************************\n");
		sleep(1);
	}
	static int init_flag;
	init_flag = 0;
	static struct buffer  buffStream;
	static struct mad_decoder decoder;
	int ttsSockfd = 0,length = -1,i = 0,index = 0,isfind = 0,retVal = 0;
	struct hostent *ttsHost;
	ttsHost = gethostbyname(httpName);	
	//	printf("address type: %s \n",(ttsHost->h_addrtype==AF_INET)?"AF_INET":"AF_INET6");
	char * ipAddr = inet_ntoa(*(struct in_addr*)ttsHost->h_addr_list[0]);
	ttsSockfd = socketCreate(ipAddr,80);
	*(framInfo->sockfdPre) = ttsSockfd;
	char * ttsBuf = getTtsBuf(text,GET_HOST,GET_HTTP);
	retVal = write(ttsSockfd,ttsBuf,strlen(ttsBuf));
	char url[1024];
	memset(url,'\0',1024);
	char line[2049];
	char * retBuff = NULL;
	char * resFree = NULL;
	int sumBufferLength = 0;
	while ((length = read(ttsSockfd,url,1)) == 1) {
		printf("%c",url[0]);
		if (i < 4) {
			if (resFree == NULL) {
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
				if (isfind == 1 && resFree == NULL) {
					line[index] = '\0';
					sumBufferLength = atoi(line);
					retBuff = (char *)malloc(sumBufferLength);
					resFree = retBuff;
				}
				index = 0;
				i++;
			} else {
				i = 0;
			}
		} else {
			printf("FreamInfo:	%p\n "
			"init_pcm_flag:	%d\n"
			"control_flag:    %d\n"
			"ending_flag:	 %d\n"
			"sockfdPre:		%d\n"
			"  buffer addr		%p\n",
			framInfo,*(framInfo->init_pcm_flag),
			*(framInfo->control_flag),*(framInfo->ending_flag),
			*(framInfo->sockfdPre),&buffStream);

			
			length = 0;
			*(framInfo->over_cmd_flag) = 1;
			while(length+1 < sumBufferLength)
			{
				retVal = recv(ttsSockfd,retBuff,BASE_BUF*4,0);
//				retVal = read(ttsSockfd,retBuff,BASE_BUF*4);
				if(retVal <= 0)
					break;
				framInfo->frame_data = retBuff;
				framInfo->frame_length = retVal;
				framInfo->pushBuff(&retBuff,&retVal,0);
			if(init_flag==0)
				downDataPlay(framInfo,&buffStream,&decoder,&init_flag);
				retBuff += retVal;
				length += retVal;
			}
			printf("end reading data from the https @@@@@@@@@@@@@	%d\n",length);
			init_flag = -1;
			downDataPlay(framInfo,&buffStream,&decoder,&init_flag);	
			if(ttsSockfd > 1)
				close(ttsSockfd);
			free(ttsBuf);
			free(resFree);
			return;
		}
	}
}


void dataBoradCast(char *httpName,char * GET_HOST,char * GET_HTTP,char *text,char **buffer,int *bufferSize,struct decode_frame*framInfo) 
{
	int ttsSockfd = 0,length = -1,i = 0,index = 0,isfind = 0,retVal = 0;
	struct hostent *ttsHost;
	ttsHost = gethostbyname(httpName);	
		printf("address type: %s \n",(ttsHost->h_addrtype==AF_INET)?"AF_INET":"AF_INET6");
	char * ipAddr = inet_ntoa(*(struct in_addr*)ttsHost->h_addr_list[0]);
	ttsSockfd = socketCreate(ipAddr,80);
	char * ttsBuf = getTtsBuf(text,GET_HOST,GET_HTTP);
	retVal = write(ttsSockfd,ttsBuf,strlen(ttsBuf));
	char url[1024];
	memset(url,'\0',1024);
	char line[2049];
	*buffer = NULL;
	char *buffKeep = (char*)malloc(1024*100);
	while ((length = recv(ttsSockfd,url,1,0)) == 1) {
		if(isfind)
		{
			buffKeep[i++]=url[0];
			if(url[0]== 'T')
				index = 0;
			line[index++] = url[0];
			if(strncmp(line,"ransfer-Encoding: chunked",strlen("ransfer-Encoding: chunked"))==0)
				i = 0;			
		}
		else{
			line[index++] = url[0];
			if(strncmp(line,"onnection: close",strlen("onnection: close"))==0)
			{
				isfind = 1;
			}
			if (url[0] ==  'C')
			{
				index = 0;
			}
		}
	}
	*bufferSize = i;
	printf("bufferSize	%d\n",*bufferSize);
	*buffer = (char*)malloc(*bufferSize);
	memcpy(*buffer,buffKeep,*bufferSize);
	free(buffKeep);
}


void MP3BoradCast(char *sAddr,struct decode_frame*framInfo)
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
	while((*sAddr != '\n') && (*sAddr != '\0'))
	{
		if(*sAddr == '/')
			break;
		buff_HTTP[i++] = *sAddr++;
	}
	sprintf(buff_HOST,"%s%s%s","HOST: ",buff_HTTP,"\r\n");
	dataBoradPlay(buff_HTTP,buff_HOST,buff_GET,"",framInfo);
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

void musicResultPlaying(cJSON * cjCont,struct decode_frame*framInfo)
{
	int i = 0, size = 0;
	cJSON * cjsCmdResult = cJSON_GetObjectItem(cjCont,"resultMusic");
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
			*(framInfo->control_flag) = PAUSE_RESUME_PALY;
			MP3BoradCast(sgUrl,framInfo);
//			*pause_resume_flag = PAUSE_RESUME_PALY;
			free(sgUrl);
			free(sgName);
			free(sgrName);
			sgUrl = sgName = sgrName = NULL;
			break;
		}
	}
}

void musicInfoPlaying(cJSON * musicData,char **mp3Data,int *mp3Size,struct decode_frame*framInfo)
{
	int i = 0;
	cJSON * musicInfo = cJSON_GetObjectItem(musicData,"musicInfo");
	cJSON * ContInfo = cJSON_GetObjectItem(musicInfo,"content");
	if(ContInfo->type == cJSON_Array)
	{
		cJSON *tNode = NULL;
		int size = cJSON_GetArraySize(ContInfo);
		for(i=0;i<size;i++)
		{
			tNode = cJSON_GetArrayItem(ContInfo,i);
			if(tNode->type == cJSON_Object)
			{
				cJSON * songsAddr = cJSON_GetObjectItem(tNode,"songSourceAddr");
				char * sourceAddress = cJSON_Print(songsAddr);
				printf("sourceAddress : %s\n",sourceAddress);
				*(framInfo->control_flag) = PAUSE_RESUME_PALY;
				MP3BoradCast(sourceAddress,framInfo);
//				*pause_resume_flag = PAUSE_RESUME_PALY;
				free(sourceAddress);		
				sourceAddress = NULL;
				break;
			}
		}	
	}

}


void mediaPauseControl(cJSON * musicData,int *pause_resume_flag)
{
	cJSON * operaName = cJSON_GetObjectItem(musicData,"operationName");
	char * operaStr = cJSON_Print(operaName);
	if(!strncmp(operaStr,"\"mediaPause\"",strlen("\"mediaPause\"")))
		*pause_resume_flag = PAUSE_RESUME_PAUSE;
	if(!strncmp(operaStr,"\"mediaResume\"",strlen("\"mediaResume\"")))
		*pause_resume_flag = PAUSE_RESUME_RESUME;
	if(!strncmp(operaStr,"\"mediaStop\"",strlen("\"mediaStop\"")))
		*pause_resume_flag = PAUSE_RESUME_STOP;
	if(!strncmp(operaStr,"\"mediaVolumeDecrease\"",strlen("\"mediaVolumeDecrease\"")))
		*pause_resume_flag = DOWN_VOICE;
	if(!strncmp(operaStr,"\"mediaVolumeIncrease\"",strlen("\"mediaVolumeIncrease\"")))
		*pause_resume_flag = UPPER_VOICE;
	if((!strncmp(operaStr,"\"mediaNext\"",strlen("\"mediaNext\"")))\
		||(!strncmp(operaStr,"\"mediaPrevious\"",strlen("\"mediaPrevious\""))))
		*pause_resume_flag = CHANGE_SONGS;
	free(operaStr);
	operaStr = NULL;
}


void jsonDataDealing(char * jsonBuf,char **mp3Data,int *mp3Size,struct decode_frame*framInfo)
{
//	printf("jsonBUF@@@@@@@@@@@@@\n%s\n",jsonBuf);
	int size = 0,i = 0;
	char * MUSIC_INFO = "resultMusic";
	cJSON * cjsRoot = cJSON_Parse(jsonBuf);
	char * cRoot = cJSON_Print(cjsRoot);
	printf("%s\n",cRoot);
	cJSON * cjsCont = cJSON_GetObjectItem(cjsRoot,"content");
	char * strCont = cJSON_Print(cjsCont);
	if(!isCharIn(strCont,MUSIC_INFO,strlen("resultMusic")))
	{
		musicResultPlaying(cjsCont,framInfo);
	}else{
		cJSON * cjsCmdType = cJSON_GetObjectItem(cjsCont,"cmdType");
//		cJSON * cjsCmdData = cJSON_GetObjectItem(cjsCont,"cmdData");
		char * cmdType = cJSON_Print(cjsCmdType);
		if(!strncmp(cmdType,"\"chat\"",6))
		{ 
			char * cmdData = NULL;
			if(isCharIn(strCont,"cmdData",strlen("cmdData")))
			{
				cJSON * cjsNlpParams = cJSON_GetObjectItem(cjsCont,"nlpParams");
				cJSON * charAnswer = cJSON_GetObjectItem(cjsNlpParams,"chatAnswer");
				cmdData = cJSON_Print(charAnswer);
			}else{
				cJSON * cjsCmdData = cJSON_GetObjectItem(cjsCont,"cmdData");
				cJSON * cmdFilter = cJSON_GetArrayItem(cjsCmdData,0);
				cmdData = cJSON_Print(cmdFilter);
			}
			printf("%s\n",cmdData);
			*(framInfo->control_flag) = PAUSE_RESUME_PALY;
			dataBoradCast(HTTP_BAIDU,BAIDU_HOST,BAIDU_HTTP,cmdData,mp3Data,mp3Size,framInfo);
			free(cmdData);			
			cmdData = NULL;
		}else if(!strncmp(cmdType,"\"media\"",7)){
			cJSON * cjsCmdData = cJSON_GetObjectItem(cjsCont,"cmdData");
			cJSON * musicData = cJSON_GetObjectItem(cjsCmdData,"data");
			cJSON * cjSubType = cJSON_GetObjectItem(cjsCmdData,"commandType");
			char * subTypeStr = cJSON_Print(cjSubType);
			if((!strncmp(subTypeStr,"\"CommandJoke\"",strlen("\"CommandJoke\"")))\
			||(!strncmp(subTypeStr,"\"CommandStoryHezi\"",strlen("\"CommandStoryHezi\""))))
			{
				musicInfoPlaying(musicData,mp3Data,mp3Size,framInfo);
			}else if(!strncmp(subTypeStr,"\"CommandMediaControl\"",strlen("\"CommandMediaControl\""))){
				mediaPauseControl(musicData,framInfo->control_flag);
			}else if(!strncmp(subTypeStr,"\"CommandSystemVolume\"",strlen("\"CommandSystemVolume\""))){
				cJSON * cjsNLP = cJSON_GetObjectItem(cjsCont,"nlpParams");
				cJSON * VLevel = cJSON_GetObjectItem(cjsNLP,"VolumnLevel");
				char * Lvalue = cJSON_Print(VLevel);
				if(strncmp(Lvalue,"\"憓?\"",strlen("\"憓?\""))!=0)
				{
					mediaPauseControl(musicData,framInfo->control_flag);
				}
				free(Lvalue);
				Lvalue = NULL;
			}
			free(subTypeStr);
			subTypeStr = NULL;		
		}
		free(cmdType);
		cmdType = NULL;
	}
	free(strCont);
	cJSON_Delete(cjsRoot);
	strCont = NULL;
}

int dataFilter(char **dataStr,char **dataStr1)
{
	int flag = 0,cntLeft = 0,cntRight = 0;
	char * ptrMov = *dataStr;
	char * ptrBase = ptrMov;
	char * ptrBaseKeep = ptrMov;
	char * ptr1Mov = *dataStr1;
	char * ptr1Base = ptr1Mov;
	int i = 0;
	while(*ptrMov!='\0')
	{
		if((*ptrMov=='{')&&(strncmp(ptrMov,"{\"code\":\"200\"",strlen("{\"code\":\"200\""))==0))	/***		?文?仍 		***/
			break;
		if((*ptrMov=='{')&&(strncmp(ptrMov,"{\"code\":\"-1\"",strlen("{\"code\":\"-1\""))==0))
			return -1;
		ptrMov++;
	}
	while(*(ptrMov)!='\0')
	{
		if(*ptrMov=='{')
			cntLeft++;
		if(*ptrMov=='}')
			cntRight++;
		if(*(ptrMov+4)!='\0')
		{
			if(((*ptrMov=='\\')&&(*(ptrMov+1)=='\"'))\
			||((*ptrMov=='\"')&&(*(ptrMov+1)=='{'))\
			||((*ptrMov=='\"')&&(*(ptrMov+1)=='}')&&(*(ptrMov+2)=='}')&&(*(ptrMov-1)=='}')))
			{																	/***2.	?駁	"{	?? "		***/
				ptrMov = ptrMov + 1;											/***3.	?駁    "}}	???", ?∩辣4銝箔?蝖桐??? 銝剔? " ?臭?撖?"  ***/
			}
			if((*ptrMov=='f')&&(*(ptrMov+1)=='s')&&(*(ptrMov+2)=='.')&&(*(ptrMov+3)=='w'))
			{
				if(!strncmp(ptrMov,"fs.w.kugou.com",strlen("fs.w.kugou.com")))				/***	?斗?臬?瑞?嚗鈭甇?	***/
				{
					flag = 1;
				}	
			}
		}														
		*ptr1Mov++ = *ptrMov++;
		if((cntLeft >0)&&(cntLeft==cntRight))
		{
			*ptr1Mov = '\0';
			break;
		}
			
	}
	ptrMov = ptr1Base;	
	if(flag == 1)
	{
		memset(ptrBase,0,50*BASE_BUF);
		ptrBaseKeep = ptrBase;
		while(*ptrMov!='\0')																/***	?文甇?	***/
		{
			if(*(ptrMov+2)!='\0')
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
						ptrMov += 2;														/*** 	?文甇?蝏偏?? ??嚗? ***/
					}
				}
			}
			if(*(ptrMov+1)!='\0')
			{
				if((*ptrMov=='\n')&&(*(ptrMov+1)!='0'))								/***		?文瘥活撣血?摨行??	***/
				{
					do{
						ptrMov++;				
					}while(*ptrMov!='\n');
				}
			}
			*ptrBase++ = *ptrMov++;
		}
		
		ptrMov = ptrBaseKeep;
	}
	memcpy(*dataStr,ptrMov,strlen(ptrMov)+1);
	return 1;
}


void cmdPlayThread(char *quesCmd,char **mp3Data,int *mp3Size,struct decode_frame*framInfo)
{
	printf("cmd	$$$$$		%s\n",quesCmd);	
	int PORTS = 8098;
	char * IPADDR = "210.22.153.226";
	int retVal = 0,uSocket = 0;
	int flag = 0, i = 0;
	int countLeft = 0,countRight = 0;
	char * postBuf = (char *)calloc(sizeof(char),BIG_BUF);
	char * jsonBuf = (char *)calloc(50,BASE_BUF);
	char * jsonKeep = (char *)calloc(50,BASE_BUF);
	
	char * jsonBufBase = jsonBuf;
	char * jsonKeepBase = jsonKeep;
	char * ptrMov  = jsonBufBase;
	getPostBuf(&postBuf,quesCmd);
		PERROR((NULL == jsonBuf ? -1 : 1),"\njsonBuf calloc error");
	usleep(1000);
	uSocket = socketCreate(IPADDR,PORTS);
	if(uSocket < 0)
		printf("create Socket Error	\n");
	retVal = write(uSocket,postBuf,strlen(postBuf));
		PERROR(retVal,"\nsend question error");
	while(1)
	{
		i = 0;
		retVal = read(uSocket,jsonBuf,BASE_BUF);
			PERROR(retVal,"\nread server error");	
		while(jsonBuf[i] != '\0')
		{
			if(jsonBuf[i] == '{')
				countLeft++;
			if(jsonBuf[i] == '}')
				countRight++;
			i++;
		}
		if(retVal == 0)
			break;
		if((countLeft > 0)&&(countLeft==countRight))
			break;
		jsonBuf += retVal;
	}
	close(uSocket);
	jsonBuf = jsonBufBase;
	retVal = dataFilter(&jsonBuf,&jsonKeepBase);
		PERROR(retVal,"\nresponse message error,error code -1");		  
	jsonDataDealing(jsonBuf,mp3Data,mp3Size,framInfo);
	*(framInfo->over_cmd_flag) = 1;
	free(postBuf);
	free(ptrMov);
	free(jsonKeepBase);
	jsonKeepBase = postBuf = ptrMov = NULL;
}




void  *cmdControlThread(void * argv)
{

	struct current_frame_over *curFlag = (struct current_frame_over *)argv;

	static int sockfdPre = -1;
	static int init_flag = 1;
	static int control_flag = 0;
	static int end_flag = 1;
	static int cur_frame_flag = 1;

	int initDecodeFlag = 8;
	struct decode_frame framData;
	
	framData.init_pcm_flag = &init_flag;
	framData.control_flag = &control_flag;
	framData.ending_flag = &end_flag;
	framData.sockfdPre = &sockfdPre;
	framData.over_cmd_flag = curFlag->over_cmd_flag;
	framData.playfunc = curFlag->playfunc;
	framData.pushBuff = curFlag->pushBuff;
	framData.tts_buffer = NULL;
	framData.tts_length = 0;
	cmdPlayThread(curFlag->quesCmd,&framData.tts_buffer,&framData.tts_length,&framData);
	if((NULL != framData.tts_buffer) && (framData.tts_length == 0))
		free(framData.tts_buffer);
	if((NULL != framData.tts_buffer) &&(framData.tts_length != 0))
	{
		struct buffer buffer;
		struct mad_decoder decoder;
		decode(&framData,&buffer,&decoder,&initDecodeFlag);
	}
		
}



void robotCmdResponse(char *quesCmd,void (*playfunc)(struct play_params *),void (*pushBuff)(char**,int*,int))
{

	struct current_frame_over currentFalg;
	static int  over_flag;
	over_flag = 0;
	currentFalg.quesCmd = quesCmd;
	currentFalg.over_cmd_flag = &over_flag;
	currentFalg.playfunc = playfunc;
	currentFalg.pushBuff = pushBuff;
	pthread_t controlThread;
    pthread_attr_t  attrControl;
    pthread_attr_init(&attrControl);
    pthread_attr_setdetachstate(&attrControl,PTHREAD_CREATE_DETACHED);
//	if(pthread_create(&controlThread,&attrControl,cmdControlThread,(void*)&currentFrame)!=0)
	if(pthread_create(&controlThread,&attrControl,cmdControlThread,(void*)&currentFalg)!=0)
   		PERROR(-1,"\npthread_create error");	
	while(over_flag == 0);
	
}	

void print_func(char * resBuf,char**resOut)
{
	strcpy(*resOut,resBuf);
}

void  *recordThread(void * argv)
{
	char ** resOut = argv;
	record_recognition(print_func,resOut);
}

#if 1
int main()
{
#if 0
	char *resOut =(char*)calloc(1,1024); 
	memset(resOut,0,1024);
	pthread_t arecordThread;
    pthread_attr_t  arecordControl;
    pthread_attr_init(&arecordControl);
    pthread_attr_setdetachstate(&arecordControl,PTHREAD_CREATE_DETACHED);
	if(pthread_create(&arecordThread,&arecordControl,recordThread,(void*)&resOut)!=0)
   		PERROR(-1,"\npthread_create error");	
#endif
	char buffS[1024]="";
	while(1)
	{
#if 0	
		if(strlen(resOut) != 0)
		{
			printf("CPY------------------------------	%s\n",resOut);
			robotCmdResponse(resOut,playMusic,fflushCommonBuffer);
			memset(resOut,0,1024);
		}
#endif

#if 1
		printf(" >>   ");
		memset(buffS,0,1024);
		scanf("%s",buffS);
		robotCmdResponse(buffS,playMusic,fflushCommonBuffer);
#endif	
	}
}
#endif

