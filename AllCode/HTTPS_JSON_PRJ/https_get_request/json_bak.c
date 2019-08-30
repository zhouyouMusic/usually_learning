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



void getPostBuf(char **transBuf,char *body,char **ip)
{
	struct hostent *ttsHost;
	ttsHost = gethostbyname("vop.baidu.com");	
	char * ipAddr = inet_ntoa(*(struct in_addr*)ttsHost->h_addr_list[0]);
	printf("IP	-	%s\n",ipAddr);
	*ip = ipAddr;
	char * pPostBuf = *transBuf;
	sprintf(pPostBuf,"%s%s%s%s%s%s%s%s%s%s",
			"POST ","/server_api?lan=zh&cuid=18:66:da:2f:dc:1f&token=23.dd43a82fbeaaeee41a8edf7ce514cef1.2592000.1513231500.514712337-10357154"," HTTP/1.1\n",
			"HOST: ",ipAddr,":443\n",
			"Content-Type: audio/amr; rate=8000\n",
			body,
			"\r\n",
			"\r\n"
			);
	*transBuf = pPostBuf;
	printf("%s",pPostBuf);
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




int main()
{
	int i = 0;
	FILE * fp = fopen("borad.pcm","rb");
	FILE * fp1 = fopen("mj.pcm","wb");
	fseek(fp,0,SEEK_END);
	long leng = ftell(fp);
	fseek(fp,0,SEEK_SET);	
	char rsp_buf[2048] = {0};
	int sockfd = 0,retVal = 0;
	char * resPonse;
	int retSize;
	char * verifyStr = (char *)calloc(33,1);
	char * bodyStr = (char *)calloc(1,leng);
	char * postStr = (char *)calloc(150,1024);
	char * jsonStr = (char *)calloc(1,1024);
	char * ipAddr = (char *)calloc(1,20);
	
	int ret = fread(bodyStr,leng,1,fp);
//	https_post_request_wait_rsp(NULL,"robot-service.centaurstech.com",443,bodyStr,rsp_buf,2024);
	getPostBuf(&postStr,bodyStr,&ipAddr);
	sockfd = socketCreate(ipAddr,443);
	sslPostHttps(postStr,sockfd,ipAddr,&resPonse,&retSize);	
}


