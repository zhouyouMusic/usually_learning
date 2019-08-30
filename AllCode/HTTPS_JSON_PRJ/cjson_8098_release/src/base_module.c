#include <stdio.h>
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

#include "baseModule.h"


int ifContainEnglish(char *quesCmd)
{
	int i = 0;
	while(quesCmd[i] != '\0')
	{
		if(((quesCmd[i]>64)&&(quesCmd[i]<91))||\
			((quesCmd[i]>96)&&(quesCmd[i]<123)))
			return -1;
		i++;
	}	
	return 0;
}

void getPostBuf(char **buff,char*quesCmd,char *ipAddr,int port)
{
	char POST_HOST[50] = "";
	sprintf(POST_HOST,"%s%s%s%d%s","HOST: ",ipAddr,":",port,"\n");
//	char * POST_HOST = "HOST: 210.22.153.226:8098\n";
	char * POST_HTTP = "POST /platform/test/test?query=";
	char * POST_TYPE = "Content-Type: application/x-www-form-urlencoded\n";
	char * TYPE_OTHER = "&type=other";
	char   quesBuf[1024];
	char   headBuf[1200];
	char * postBuf = *buff;
	if(NULL==postBuf)
	{
		perror("postBuf  error its Null\n");
		return;
	}
	int retVal = 0;
	strcpy(quesBuf,quesCmd);
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
	*buff = postBuf;
}

int contentIsUtf8(char *str) 
{     
	int i = 0;     
	int size = strlen(str);       
	while(i < size)     
	{         
		int step = 0;         
		if((str[i] & 0x80) == 0x00)        
		{             
			step = 1;         
		}else if((str[i] & 0xe0) == 0xc0)         
		{             
			if(i + 1 >= size) 
				return false;             
			if((str[i + 1] & 0xc0) != 0x80) 
				return false;               
			step = 2;         
		}else if((str[i] & 0xf0) == 0xe0)         
		{             
			if(i + 2 >= size) 
				return false;             
			if((str[i + 1] & 0xc0) != 0x80) 
				return false;             
			if((str[i + 2] & 0xc0) != 0x80) 
				return false;               
			step = 3;        
		}else  
		{             
			return false;         
		}          
		i += step;    
	}       
	if(i == size) 
		return true;      
	return false; 
}


int socketCreate(char * ipAddress,int port)
{
	int uSocket = 0;
	uSocket = socket(AF_INET,SOCK_STREAM,0);
	if(uSocket <= 0)
	{
		BASE_ERROR_LOG(ErrLogHandle,"%s"," failed to create socket\n");
		return -1;
	}
	struct sockaddr_in sockAddr;
	memset(&sockAddr,0,sizeof(struct sockaddr_in));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(port);
	if(inet_pton(AF_INET,ipAddress,&sockAddr.sin_addr) <= 0)
	{
		BASE_ERROR_LOG(ErrLogHandle,"%s","failed to convert 	IPADDR to internet IP\n");
		return -1;
	}	
	if(connect(uSocket,(struct sockaddr *)&sockAddr,sizeof(struct sockaddr_in)) < 0)
	{
		BASE_ERROR_LOG(ErrLogHandle,"%s","failed to connect to the server\n");
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


