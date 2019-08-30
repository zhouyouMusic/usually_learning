#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<string.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<pthread.h>
#include<signal.h>
#include<time.h>
#include<errno.h>
#include<memory.h>
#include<unistd.h>
#include<netdb.h>
#include<unistd.h>
#include<time.h>
#include <sys/time.h>
#include "libusc.h"
#include "cloud_sounds_recognition.h"


void *speech_request(void *arg)
{
	int clientFd = arg;
	char buffer[2048];
	while(1)
	{
		memset(buffer,0,2048);
		int retVal = recv(clientFd,buffer,2048,0);
		if(retVal < 1)
			break;
		printf("buffer	%s\n",buffer);
		send(clientFd,"i am 007",20,0);
	}
	close(clientFd);

}


void build_server()
{
	InfoLogHandle = logInitHandle(FILE_NAME,FILE_PATH,MAX_SIZE,KEEP_NUM);
	ErrLogHandle = logInitHandle(ERROR_FILE,FILE_PATH,MAX_SIZE,KEEP_NUM);
	
	pthread_mutex_init(&MUTEX_CLIENT,NULL);
	int err = 0;
	decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &err);
		
    int server_fd = 0,client_fd = 0;
    struct sockaddr_in address;
    memset(&address,0,sizeof(address));
    server_fd = socket(AF_INET,1,0);
	
    address.sin_family = AF_INET;
    address.sin_port = htons(8996);
    address.sin_addr.s_addr = INADDR_ANY;
    if(bind(server_fd,(struct sockaddr*)&address,sizeof(address))<0)
    {
        perror("service bind error");
        exit(1);
    }
    if(listen(server_fd,1024) < 0)
    {
        perror("service listen error");
        exit(1);
    }
    while(1)
    {
        client_fd = accept(server_fd,NULL,NULL);
		usleep(10000);
        if(client_fd < 0)
        {
            printf("退出\n");
			BASE_ERROR_LOG(ErrLogHandle,"%s","client fd is  connect error\n");
            return;
        }else{
			BASE_INFO_LOG(InfoLogHandle,"%s","*********************clinet connected ******\n");
		pthread_mutex_lock(&MUTEX_CLIENT);
		    pthread_t thread;
            pthread_attr_t  attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
            pthread_create(&thread,&attr,speech_request,(void*)client_fd);
		pthread_mutex_unlock(&MUTEX_CLIENT);
        }
    }
    return;
}
int main()
{
	build_server();
}

