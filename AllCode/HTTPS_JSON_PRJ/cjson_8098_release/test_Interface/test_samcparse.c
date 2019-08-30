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
#include "cmdReqInterface.h"

int main()
{
	char *smdRlt = (char *)malloc(1024);
	SemcParseHandle semcParseHandTemp = semcParseHandInit();
	char buffS[1024] = "";

	while(1)
	{	
		printf("enter Cmd	: ");
		scanf("%s",buffS);
		if(strlen(buffS) != 0)
		{
			semcParseResponse(buffS,semcParseHandTemp,&smdRlt);
			printf("%s\n",smdRlt);
			memset(buffS,0,1024);
			usleep(1000);
		}
	}
}




























