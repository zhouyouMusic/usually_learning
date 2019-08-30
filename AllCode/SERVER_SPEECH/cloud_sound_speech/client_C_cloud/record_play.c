#include <stdio.h>
#include "alsaInterface.h"
#include "myrecord_android.h"
int main()
{

	AlsaHandle recordHandle;
	recordHandle = alsaInit("2,0",0,2,48000,0);
	alsaStart(recordHandle);
#if 1
	FILE * fp = fopen("test.wav","rb");
	fseek(fp,44,SEEK_SET);
	char buff[2048] = "";
	int ret;
	while(1)
	{
		ret = fread(buff,1,2048,fp);
		if(ret < 1)
			break;
		alsaSend(recordHandle,buff,ret);
	}
#endif
}
