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

void func_result(char *sphOut)
{
	printf("%s	\n",sphOut);
	fflush(stdout);

}


int main()
{
	char *resOut = (char *)malloc(1024);
	HwSoundHandle uniSdHandle = hwSoundHandleInit("192.168.1.116",8999,func_result);	
	hwSoundRecgStart(uniSdHandle);

	FILE * fp = fopen("test.pcm","rb");
	char feedBuff[2048];
	int ret = 0;
	while(1)
	{
		memset(feedBuff,0,2048);
		ret = fread(feedBuff,1,2048,fp);
		if(ret < 1)
		{
			hwSoundRecgStop(uniSdHandle);
			break;
		}
		hwSoundFeedData(uniSdHandle,feedBuff,2048);
	}
	hwSoundDestroy(uniSdHandle);
	while(1);
}


