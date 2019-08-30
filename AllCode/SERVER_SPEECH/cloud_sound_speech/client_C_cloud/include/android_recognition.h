#ifndef ANDROID_RECOGNITION
#define ANDROID_RECOGNITION

struct post_content
{
	int contType;
	int contLength;
	char buff[2048];
};

struct get_content
{
	int socket_fd;
	char **cloudResult;
	void (*resultFun)(char *,char **);
};

int writeRecStart(int sockfd);

int writeRecStop(int sockfd);

int connectServer(char *ipaddr,int port);

void *pthread_showCont(void *arg);

void create_readFunc(int sockfd,struct get_content *getParams,void (*funcResult)(char*,char **),char**resOut);


#endif
