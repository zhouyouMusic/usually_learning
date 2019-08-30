#include <stdio.h>
//#include "alsaPlay.h"
#include "alsaInterface.h"
int main(int argc,char **argv)
{
	printf("%s %s %s %s\n",argv[1],argv[2],argv[3],argv[4]);
	AlsaHandle handle = alsaInit(argv[1],0,atoi(argv[3]),atoi(argv[4]),0);
	FILE *fp = fopen("./test.wav","rb");
	char buffer[1024];
	int length = -1;
	alsaStart(handle);
	while((length = fread(buffer,1,1024,fp)) > 0)
	{
		alsaSend(handle,buffer,1024);
		printf("hello world\n");
		fflush(stdout);
		sleep(1);
	}
	alsaStop(handle);
}
