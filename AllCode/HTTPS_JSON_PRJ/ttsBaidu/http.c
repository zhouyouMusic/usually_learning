#include "http.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include "http.h"
#include <unistd.h>
#include <stdlib.h>

extern void getSocket(char *text,char **buffer,int *bufferSize) {
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in servaddr;
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	struct hostent *host = gethostbyname(URL);
	servaddr.sin_addr =*((struct in_addr *)host->h_addr);
	connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
	char url[1024];
	memset(url,'\0',1024);
	sprintf(url,"%s%s%s%s%s%s",HTTP_GET,text,HTTP_HOST,HTTP_CONTENT,HTTP_ACCEPT,HTTP_END);
	write(sockfd,url,strlen(url));
	int length = -1;
	int i = 0;
	int index = 0;
	char line[2049];
	int isfind = 0;
	*buffer = NULL;
	while ((length = read(sockfd,url,1)) > 0) {
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
	close(sockfd);
}


int main(int argc,char **argv)
{
	char *bufAddr;// = (char*)calloc(1,9000);
	int bufferSize  = 0;	
	getSocket(argv[1],&bufAddr,&bufferSize);
	printf("%d\n",bufferSize);
	FILE *fp = fopen("./test.mp3","wb");
	fwrite(bufAddr,bufferSize,1,fp);
	fclose(fp);
	return 0;
}
	
