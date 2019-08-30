#include <stdio.h>
#include "alsaInterface.h"
int main () 
{
//	AlsaHandle handle = alsaInit("plug:hw:2",1,12,16000,0);
	AlsaHandle handle = alsaInit("plughw:1,0",1,12,16000,0);
	char buffer[1024 * 10];
	int length = -1;
	FILE *fp = fopen("./test.wav","wb");
	int number = 300;
	while ((length = alsaRead(handle,buffer,1024 * 10)) > 0) 
	{
		fwrite(buffer,length,1,fp);
		if (number-- < 0) {
			break;
		}
	}
	fclose(fp);
	alsaDestroy(handle);
}
