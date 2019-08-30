#include <stdio.h>
#include <fcntl.h>
#include <setjmp.h> 
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
#include <sys/time.h>
#include <alsa/asoundlib.h>
#include <linux/soundcard.h>
#include "mad.h"
#include "semcParse.h"
#include "madAlsaPlay.h"
#include "baseModule.h"
#include "JSON_checker.h"
#include "r16_gpio_control.h" 


int MP3BoradCast(char *sAddr,SemcParseHandle smcPreHand)
{
	char buff_GET[200] = "";
	char buff_HOST[50] = "";
	char buff_HTTP[50] = "";
	char buff_MP3Post[512] = "";
	int i = 0;
	sAddr = sAddr + 1;
	*(sAddr + strlen(sAddr)-1) = '\0';
	sprintf(buff_GET,"%s%s","GET ",sAddr);
	BASE_INFO_LOG(InfoLogHandle,"%s%s",sAddr,"\n");
	sAddr = sAddr + strlen("http://");
	while((*sAddr != '\n') && (*sAddr != '\0'))
	{
		if(*sAddr == '/')
			break;
		buff_HTTP[i++] = *sAddr++;
	}
	sprintf(buff_HOST,"%s%s%s","HOST: ",buff_HTTP,"\r\n");
	sprintf(buff_MP3Post,"%s%s%s%s%s%s%s",
			buff_GET,
			"",
	 		" HTTP/1.1\r\n",
	 		buff_HOST,
	 		"Connection: close\r\n",
	 		"Accept: */*\r\n",
	 		"\r\n"
			 );
	SmdParsingBuffer* smcPreHandle = (SmdParsingBuffer*)smcPreHand;
	strcpy(smcPreHandle->buff_HTTP,buff_HTTP);
	strcpy(smcPreHandle->req_Content,buff_MP3Post);
//printf("%s %s	\n",smcPreHandle->buff_HTTP,smcPreHandle->req_Content);
	if(strlen(sAddr) > 10)
		return 0;
	return -1;
}



void musicResultPlaying(char * quesBuf,SemcParseHandle smcPreHand)
{
	int i = 0,j = 0;
	char httpBuff[50] = "";
	char  sgUrl[200] = "";
	while(quesBuf[i] != '\0')
	{
		if(quesBuf[i] == '\"')
		{
			memset(httpBuff,0,50);
			strncpy(httpBuff,quesBuf+i,50);
			httpBuff[49] = '\0';
			if(strncmp(httpBuff,"\"http://fs.w.kugou.com",strlen("\"http://fs.w.kugou.com"))==0)
			{
				sgUrl[j++] = quesBuf[i++];
				while(quesBuf[i] != '\"')
				{
					sgUrl[j++] = quesBuf[i++];
				}
				sgUrl[j++] = quesBuf[i];
				sgUrl[j] = '\0';
				break;
			}
		}
		i++;
	}	
	SmdParsingBuffer* smcPreHandle = (SmdParsingBuffer*)smcPreHand;
	strcpy(smcPreHandle->ctlCmdJSON,MEDIA_URL);
	strcat(smcPreHandle->ctlCmdJSON,sgUrl);
}


void musicInfoPlaying(cJSON * musicData,SemcParseHandle smcPreHand)
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
				BASE_INFO_LOG(InfoLogHandle,"%s%s%s","sourceAddress : ",sourceAddress,"\n");		
				SmdParsingBuffer* smcPreHandle = (SmdParsingBuffer*)smcPreHand;
				strcpy(smcPreHandle->ctlCmdJSON,MEDIA_URL);
				strcat(smcPreHandle->ctlCmdJSON,sourceAddress);
				free(sourceAddress);		
				sourceAddress = NULL;
				break;
			}
		}	
	}
}

void jsonDataDealing(char * jsonBuf,SemcParseHandle smcPreHand)
{
	SmdParsingBuffer* smcPreHandle = (SmdParsingBuffer*)smcPreHand;
	memset(smcPreHandle->ctlCmdJSON,0,1024);
	memset(smcPreHandle->buff_HTTP,0,50);
	memset(smcPreHandle->req_Content,0,4096);

	char * MUSIC_INFO = "resultMusic";
	if(!isCharIn(jsonBuf,MUSIC_INFO,strlen("resultMusic")))
	{
		musicResultPlaying(jsonBuf,smcPreHand);
		return;
	}
	if(0!=json_checker(jsonBuf,20))
	{
		BASE_ERROR_LOG(ErrLogHandle,"%s","json format checking error\n");
		char * elseCmd = "\"您好,本条数据获取失败,请稍后再试\"";
		strcpy(smcPreHandle->ctlCmdJSON,TTS_TEXT);
		strcat(smcPreHandle->ctlCmdJSON,elseCmd);
		return;
	}
	cJSON * cjsRoot = cJSON_Parse(jsonBuf);
	char * cRoot = cJSON_Print(cjsRoot);
//		printf("%s\n",cRoot);
	cJSON * cjsCont = cJSON_GetObjectItem(cjsRoot,"content");
	char * strCont = cJSON_Print(cjsCont);
	cJSON * cjsCmdType = cJSON_GetObjectItem(cjsCont,"cmdType");
	char * cmdType = cJSON_Print(cjsCmdType);
	if(!strncmp(cmdType,"\"chat\"",6))
	{ 
		char * cmdData = NULL;
		if(isCharIn(strCont,"cmdData",strlen("cmdData")))
		{
			cJSON * cjsNlpParams = cJSON_GetObjectItem(cjsCont,"nlpParams");
			if(isCharIn(strCont,"chatAnswer",strlen("chatAnswer")))
			{
				char * elseCmd = "\"您好,本条数据获取失败,请稍后尝试\"";
				strcpy(smcPreHandle->ctlCmdJSON,TTS_TEXT);
				strcat(smcPreHandle->ctlCmdJSON,elseCmd);
				return;	
			}else{
				cJSON * charAnswer = cJSON_GetObjectItem(cjsNlpParams,"chatAnswer");
				cmdData = cJSON_Print(charAnswer);
			}
		}else{
			cJSON * cjsCmdData = cJSON_GetObjectItem(cjsCont,"cmdData");
			cJSON * cmdFilter = cJSON_GetArrayItem(cjsCmdData,0);
			cmdData = cJSON_Print(cmdFilter);
		}
		if((strcmp("\"主人，还不理解您说的呢\"",cmdData)==0)||(strcmp("\"主人，小哈还不会呢\"",cmdData)==0))
		{
			char * elseCmd = "\"哈哈哈哈,这个我还在学习中,请给一点时间\"";
			strcpy(smcPreHandle->ctlCmdJSON,TTS_TEXT);
			strcat(smcPreHandle->ctlCmdJSON,elseCmd);
		}else{
			strcpy(smcPreHandle->ctlCmdJSON,TTS_TEXT);
			strcat(smcPreHandle->ctlCmdJSON,cmdData);	
		}
		free(cmdData);			
		cmdData = NULL;
	}else if(!strncmp(cmdType,"\"media\"",7)){
		cJSON * cjsCmdData = cJSON_GetObjectItem(cjsCont,"cmdData");
		cJSON * musicData = cJSON_GetObjectItem(cjsCmdData,"data");
		cJSON * cjSubType = cJSON_GetObjectItem(cjsCmdData,"commandType");
		char * subTypeStr = cJSON_Print(cjSubType);
		if((!strncmp(subTypeStr,"\"CommandJoke\"",strlen("\"CommandJoke\"")))\
		||(!strncmp(subTypeStr,"\"CommandStoryHezi\"",strlen("\"CommandStoryHezi\"")))\
		||(!strncmp(subTypeStr,"\"CommandMusic\"",strlen("\"CommandMusic\""))))
		{
			musicInfoPlaying(musicData,smcPreHand);
		}else if(!strncmp(subTypeStr,"\"CommandMediaControl\"",strlen("\"CommandMediaControl\""))){
			cJSON * operaName = cJSON_GetObjectItem(musicData,"operationName");
			char * operaStr = cJSON_Print(operaName);
			strcpy(smcPreHandle->ctlCmdJSON,"media_");
			strcat(smcPreHandle->ctlCmdJSON,"CommandMediaControl_");
			strcat(smcPreHandle->ctlCmdJSON,operaStr);
			free(operaStr);	
		}else if(!strncmp(subTypeStr,"\"CommandSystemVolume\"",strlen("\"CommandSystemVolume\""))){
			cJSON * cjsNLP = cJSON_GetObjectItem(cjsCont,"nlpParams");
			cJSON * VLevel = cJSON_GetObjectItem(cjsNLP,"VolumnLevel");
			char * Lvalue = cJSON_Print(VLevel);		
			strcpy(smcPreHandle->ctlCmdJSON,"media_");
			strcat(smcPreHandle->ctlCmdJSON,"CommandSystemVolume_");
			strcat(smcPreHandle->ctlCmdJSON,Lvalue);
			free(Lvalue);
		}
		free(subTypeStr);
		subTypeStr = NULL;		
	}else if(!strncmp(cmdType,"\"homeControl\"",strlen("\"homeControl\""))){
			cJSON * cjsNLP = cJSON_GetObjectItem(cjsCont,"nlpParams");
			char * chNLP = cJSON_Print(cjsNLP);
			if(!isCharIn(chNLP,"Operation",strlen("Operation")))
			{
				cJSON * oPrtn = cJSON_GetObjectItem(cjsNLP,"Operation");
				char  * opName = cJSON_Print(oPrtn);
				cJSON * fCly = cJSON_GetObjectItem(cjsNLP,"Facility");
				char  * fyName = cJSON_Print(fCly);
				strcpy(smcPreHandle->ctlCmdJSON,"homeControl_");
				strcat(smcPreHandle->ctlCmdJSON,opName);
				strcat(smcPreHandle->ctlCmdJSON,"_");
				strcat(smcPreHandle->ctlCmdJSON,fyName);
				free(opName);
				free(fyName);
			}else{
				strcpy(smcPreHandle->ctlCmdJSON,"homeControl_");
				strcat(smcPreHandle->ctlCmdJSON,"x");
				strcat(smcPreHandle->ctlCmdJSON,"_");
				strcat(smcPreHandle->ctlCmdJSON,"x");
			}
			free(chNLP);
	}else{
			char * elseCmd = "\"您好,本条数据获取失败,请稍后尝试\"";
			strcpy(smcPreHandle->ctlCmdJSON,TTS_TEXT);
			strcat(smcPreHandle->ctlCmdJSON,elseCmd);
	}		
	free(cmdType);
	cmdType = NULL;
	free(strCont);
	cJSON_Delete(cjsRoot);
	strCont = NULL;
}

int dataFilter(char **dataStr,char **dataStr1)
{
	int lyricFlag = 0,cntLeft = 0,cntRight = 0;
	int filterFlag = 0;
	char * ptrMov = *dataStr;
	char * ptrBase = ptrMov;
	char * ptrBaseKeep = ptrMov;
	char * ptr1Mov = *dataStr1;
	char * ptr1Base = ptr1Mov;
	while(*ptrMov!='\0')
	{
		if((*ptrMov=='{')&&(strncmp(ptrMov,"{\"code\":\"200\"",strlen("{\"code\":\"200\""))==0))	
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
			{																
				ptrMov = ptrMov + 1;											
			}
			if((*ptrMov=='f')&&(*(ptrMov+1)=='s')&&(*(ptrMov+2)=='.')&&(*(ptrMov+3)=='w'))
			{
				if(!strncmp(ptrMov,"fs.w.kugou.com",strlen("fs.w.kugou.com")))			
				{
					lyricFlag = 1;
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
	if(lyricFlag == 1)
	{
		memset(ptrBase,0,50*BASE_BUF);
		ptrBaseKeep = ptrBase;
		while(*ptrMov!='\0')													
		{		
			if(*(ptrMov+2)!='\0')
			{
				if((*(ptrMov+1)=='l')&&(*(ptrMov+2)=='y')&&(*ptrMov == '\"'))
				{
					
					if(!strncmp(ptrMov+1,"lyrics\":\"",strlen("lyrics\":\"")))
					{
						while(*(ptrMov+2)!='\0')
						{
							if((*ptrMov=='\"')&&(*(ptrMov+1)==','))		
								break;
							ptrMov++;	
						}																
						ptrMov += 2;												
					}
				}
				if((*ptrMov=='\n')&&(*(ptrMov+1)!='0'))							
				{
					do{
						ptrMov++;
					}while(*ptrMov!='\n');
					filterFlag = 1;
//					ptrMov=ptrMov+1;
				}
			}
			*ptrBase++ = *ptrMov++;
		}
		
		ptrMov = ptrBaseKeep;
		ptrBase = ptr1Base;

		int countMov = 0;
		int length = strlen(ptrMov);
		int indexIO,indexP,indexS;
		int i = 0,j = 0,k = 0,indexFlag = 0;
		int indexArry[10]={0};
		if(filterFlag)
		{
			for(indexP = 0;indexP < length;indexP++)
			{
				if(ptrMov[indexP] == ',')
				{
					indexIO = indexP;					
				}
				if((ptrMov[indexP] == ',')&&(indexFlag))
				{
					indexArry[i++] = indexIO;
					indexFlag = 0;
				}
				if(ptrMov[indexP] == '\n')
				{
					indexArry[i++] = indexIO;
					indexFlag = 1;
				}				
			}
			indexArry[i++] = length;
			indexArry[i++] = length+1;

			for(i = 0;i < 10;i++)
			{
				countMov = indexArry[i];
				if((countMov != 0)&&(i%2==0))
				{
					if(i==0)
					{
						j = 0;
					}else{
						j = indexArry[i-1];
					}
	//				printf("!!!!!!!!!!	%d		%d\n",countMov,j);
					for(j;j<countMov;j++)
					{
						*ptrBase++ = ptrMov[j];
					}
				}
			}
			*ptrBase = '\0';
			ptrMov = ptr1Base;
		}
		
	}
//	printf("%s\n",ptrMov);
	memcpy(*dataStr,ptrMov,strlen(ptrMov)+1);
	return 1;
}


void cmdPlayThread(SemcParseHandle smcPreHand,char **smdOut)
{	
	SmdParsingBuffer* smcPreHandle = (SmdParsingBuffer*)smcPreHand;
#if 1
	if(NULL == (*smdOut))
		return;
	if(strcmp("NoVoiceInput",smcPreHandle->quesCmdBuff)==0)
	{
		strcpy(*smdOut,TTS_TEXT);
		strcat(*smdOut,"\"未检测到声音输入\"");
        return;
	}
	if(strcmp("CLOUDSERVER_ERROR",smcPreHandle->quesCmdBuff)==0)
	{
		strcpy(*smdOut,TTS_TEXT);
		strcat(*smdOut,"\"正在重新连接服务器\"");
        return;
	}
	if(strcmp("RequestOverTime",smcPreHandle->quesCmdBuff)==0)
	{
		strcpy(*smdOut,TTS_TEXT);
		strcat(*smdOut,"\"获取云端数据超时\"");
        return;
	}
	if(ifContainEnglish(smcPreHandle->quesCmdBuff)==-1)
	{
		strcpy(*smdOut,TTS_TEXT);
		strcat(*smdOut,"\"");
		strcat(*smdOut,smcPreHandle->quesCmdBuff);
		strcat(*smdOut,"\"");
        return;
	}
#endif
	int PORTS = smcPreHandle->port;
	char * IPADDR = smcPreHandle->ipAddr;
	int retVal = 0,uSocket = 0;
	int flag = 0, i = 0;
	int countLeft = 0,countRight = 0;
	char * postBuf = smcPreHandle->postHttpReq;
	char * jsonBuf = smcPreHandle->jsonBaseBuff;
	char * jsonKeep = smcPreHandle->jsonTempBuff;
	memset(postBuf,0,1024);
	memset(jsonBuf,0,50*1024);
	memset(jsonKeep,0,50*1024);
	char * jsonBufBase = jsonBuf;
	char * jsonKeepBase = jsonKeep;
	char * ptrMov  = jsonBufBase;
	BASE_INFO_LOG(InfoLogHandle,"%s%s%s","quesCmdBuf - ",smcPreHandle->quesCmdBuff,"\n");
	getPostBuf(&postBuf,smcPreHandle->quesCmdBuff,IPADDR,PORTS);
	uSocket = socketCreate(IPADDR,PORTS);
		PERROR(uSocket,"\ncreate Socket Error	\n");
	retVal = write(uSocket,postBuf,strlen(postBuf));
		PERROR(retVal,"\nsend question error\n");
	struct timeval timeoutRcv = {5,0};
	retVal = setsockopt(uSocket,SOL_SOCKET,SO_RCVTIMEO,
							(char *)&timeoutRcv,sizeof(struct timeval));
	if(retVal < 0)
		BASE_ERROR_LOG(ErrLogHandle,"%s","setsockopt for timeout error \n");
	while(1)
	{
		i = 0;
		retVal = recv(uSocket,jsonBuf,BASE_BUF,0);
		if(retVal < 0)
		{
			BASE_ERROR_LOG(ErrLogHandle,"%s","read server error\n");
			strcpy(*smdOut,TTS_TEXT);
			strcat(*smdOut,"\"获取数据超时\"");
			return;
		}
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
	if(retVal < 0)
	{
		strcpy(*smdOut,TTS_TEXT);
		strcat(*smdOut,smcPreHandle->quesCmdBuff);
		BASE_ERROR_LOG(ErrLogHandle,"%s","response message error,error code -1\n");
		return;
	}
	jsonDataDealing(jsonBuf,smcPreHand);
	strcpy(*smdOut,smcPreHandle->ctlCmdJSON);
}



extern SemcParseHandle semcParseHandInit()
{
	SmdParsingBuffer * smdParseHandle = (SmdParsingBuffer *)malloc(sizeof(SmdParsingBuffer));

	smdParseHandle->quesCmdBuff = (char *)malloc(1024);
	smdParseHandle->postHttpReq = (char *)malloc(1024);
	smdParseHandle->jsonBaseBuff = (char *)malloc(1024 *50);
	smdParseHandle->jsonTempBuff = (char *)malloc(1024 *50);
	smdParseHandle->ipAddr = (char *)malloc(50);
	smdParseHandle->port = 8098;
	strcpy(smdParseHandle->ipAddr,"210.22.153.226");
	
	
	smdParseHandle->buff_HTTP = (char *)malloc(50);
	smdParseHandle->req_Content = (char *)malloc(1024 *4);
	smdParseHandle->ctlCmdJSON = (char *)malloc(1024);
	smdParseHandle->smdParType = WITHOUT_PALYING;
		
	sem_init(&Sem_SmdParse,0,0);
	sem_post(&Sem_SmdParse);
	return (SemcParseHandle)smdParseHandle;
}


extern void semcParseResponse(char *quesCmd,SemcParseHandle semcPrHand,char **smdRlt)
{
	sem_wait(&Sem_SmdParse);
	SmdParsingBuffer * smdParseHandle = (SmdParsingBuffer *)semcPrHand;
	memset(smdParseHandle->quesCmdBuff,0,1024);
	strncpy(smdParseHandle->quesCmdBuff,quesCmd,strlen(quesCmd)+1);
	cmdPlayThread(semcPrHand,smdRlt);
	sem_post(&Sem_SmdParse);
	BASE_INFO_LOG(InfoLogHandle,"%s%s",*smdRlt,"		&\n");
}	


