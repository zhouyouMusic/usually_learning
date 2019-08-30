#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../libs/libusc.h"
#include "../libs/appKey.h"

struct recog_params
{
	char  domain[50];
	char  audioFormat[50];
	int   IsUseNlu;
	char  NluBuff[50];
	char  appKey[100];
	char  secretKey[100];
};


void init_speech_recognition(USC_HANDLE* handle,struct recog_params * recogParam)
{
 	int ret = usc_create_service(handle);
	if (ret != USC_ASR_OK) {
		fprintf(stderr, "usc_create_service_ext error %d\n", ret);
		return;
	}
    ret = usc_set_option((*handle), USC_OPT_ASR_APP_KEY, recogParam->appKey);
    ret = usc_set_option((*handle), USC_OPT_USER_SECRET, recogParam->secretKey);
	ret = usc_login_service(*handle);
    ret = usc_set_option((*handle), USC_OPT_INPUT_AUDIO_FORMAT, recogParam->audioFormat);
    ret = usc_set_option((*handle), USC_OPT_RECOGNITION_FIELD, recogParam->domain);	
    if (recogParam->IsUseNlu)
          ret = usc_set_option((*handle), USC_OPT_NLU_PARAMETER, recogParam->NluBuff);
}

int start_speech_recognition(USC_HANDLE handle)
{
	return usc_start_recognizer(handle);
}
int stop_speech_recognition(USC_HANDLE handle,char **lastBuff)
{
	int retVal = usc_stop_recognizer(handle);
	if(retVal != 0)		return retVal;
	char buffer[680] = "";
	strcpy(buffer,usc_get_result(handle));
	
	int lalength = strlen(buffer);
	printf("@@@@@@@@@		%s\n",buffer);
	*lastBuff = (char *)malloc(sizeof(char)*(lalength+1));
	memset(*lastBuff,0,lalength+1);
	strncpy(*lastBuff,buffer,lalength+1);
	return retVal;
}
void destroy_speech_recognition(USC_HANDLE handle)
{
	usc_release_service(handle);
}


void automatic_speech_recognition(USC_HANDLE handle,char *pcmBuff,int pcmLength)
{
	int ret = usc_feed_buffer(handle,pcmBuff,pcmLength);
	if (ret == USC_RECOGNIZER_PARTIAL_RESULT ||
		ret == USC_RECOGNIZER_SPEAK_END) {	
		printf("result		%s\n",usc_get_result(handle));
	}
	else if (ret < 0) {		
		fprintf(stderr, "usc_feed_buffer error %d\n", ret);
		return;
	}
}


int main(int argc, char* argv[])
{
	USC_HANDLE handle;
	struct recog_params recogParam = {0};
	strcpy(recogParam.appKey,USC_ASR_SDK_APP_KEY);
	strcpy(recogParam.secretKey,USC_ASR_SDK_SECRET_KEY);
	strcpy(recogParam.domain,RECOGNITION_FIELD_GENERAL);
	strcpy(recogParam.audioFormat,AUDIO_FORMAT_PCM_16K);
	recogParam.IsUseNlu = 0;
	init_speech_recognition(&handle,&recogParam);


	start_speech_recognition(handle);

	FILE * fp = fopen(argv[1],"rb");
	char buffer[1024] = "";
	char * LastBuff = NULL;
	int retVal = 0;
	while(1)
	{
		retVal = fread(buffer,1,640,fp);

		if(retVal <= 0)
			break;
		automatic_speech_recognition(handle,buffer,retVal);

	}
	stop_speech_recognition(handle,&LastBuff);
	printf("!!!!!!!!!!!		%s\n",LastBuff);
//	usc_release_service(handle);
	
#if 0	
	memset(buffer,0,1024);
	start_speech_recognition(handle);
	FILE * fp2 = fopen("mj.pcm","rb");
	LastBuff = NULL;
	while(1)
	{
		retVal = fread(buffer,1,1024,fp2);

		if(retVal <= 0)
			break;
		automatic_speech_recognition(handle,buffer,retVal);

	}
	stop_speech_recognition(handle,&LastBuff);
	printf("!!!!!!!!!!!		%s\n",LastBuff);
//	usc_release_service(handle);
		
	memset(buffer,0,640);
	start_speech_recognition(handle);
	FILE * fp3 = fopen("hello.pcm","rb");
	LastBuff = NULL;
	while(1)
	{
		retVal = fread(buffer,1,640,fp3);

		if(retVal <= 0)
			break;
		automatic_speech_recognition(handle,buffer,retVal);

	}
	stop_speech_recognition(handle,&LastBuff);
	printf("!!!!!!!!!!!		%s\n",LastBuff);
#endif
}











