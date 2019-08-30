#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <alsa/asoundlib.h>
#include <linux/soundcard.h>
#include "cmdReqInterface.h"

void func_result(char *sphOut)
{
	printf("%s	\n",sphOut);

}


int main()
{
	HwSoundHandle uniSdHandle = hwSoundHandleInit("192.168.1.117",8999,func_result);	
	hwSoundRecgStart(uniSdHandle);
	FILE * fp = fopen("test.pcm","rb");
	char * feedBuff = (char *)malloc(2048);
	int ret = 0;
	int count = 0;
	while(1)
	{
		count++;
		memset(feedBuff,0,2048);
		ret = fread(feedBuff,2048,1,fp);
		if(ret < 1)
		{
			hwSoundRecgStop(uniSdHandle);
			break;
		}
		if(count == 60)	
		{
			hwSoundRecgStop(uniSdHandle);
			break;
		
		}
		hwSoundFeedData(uniSdHandle,feedBuff,2048);
	}
	printf("ending ~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	hwSoundDestroy(uniSdHandle);
	while(1);
}


