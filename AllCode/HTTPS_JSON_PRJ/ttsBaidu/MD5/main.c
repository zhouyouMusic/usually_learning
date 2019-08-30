#include <stdio.h>
#include <unistd.h>  
#include <stdlib.h>  
#include <fcntl.h>  
#include <limits.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <string.h>  
  
#include "madAlsa.h"

int main()
{
	int retVal = 0,count = 0,ret = 0;
 	char *buff = (char*)calloc(1,1024*15);
	char *tts = buff;
    FILE *fp3 = fopen("borad.mp3", "rb");  
	while(1)
	{
		retVal = fread(buff,1,1024,fp3);
		buff = buff + retVal;
		count+=retVal;
		if(retVal < 1)
			break;
		printf("%d		%s\n",retVal,buff);
	}
		boradMp3(count,tts,16000,1);
}
