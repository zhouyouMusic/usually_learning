#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "alsaInterface.h"
#include "huwenvadInterface.h"
#include "voiceInterface.h"
#include "android_recognition.h"

static void micShortToFloat(short *input,float *output,float *aec,int nMics,int length) {

        int k = 0;
        int j = 0;
        //        int length = (FRAME_LEN << 1);
        float *aec2 = aec + 1024;
        for (k = 0; k < length;k++) {
                output[k] = input[8 + 12 * k] * 0.00003052f;
                aec[k] = input[10 + 12 * k] * 0.00003052f;
                aec2[k] = input[11 + 12 * k] * 0.00003052f;
        }
        for (; j < nMics - 1;j++) {
                for (k = 0; k < length;k++) {
                        output[k + (j + 1) * length] = input[j + 12 * k] * 0.00003052f;
                }
        }
}

void record_recognition(void (*func_result)(char*,char**),char**resOut)
{	

#if 1
//	init_speech_recognition(&handleUSC,&recogParam);		
	int length = -1;
	int vadNumber = -1;
	char *pBuf = (char*)calloc(1024,10);
	int count = 0;
	
	AlsaHandle recordHandle;
	VoiceHandle voiceHandle;	
	voiceInit(&voiceHandle);
	HuwenVADHandle	vadHandle;
	huwenVadInit(&vadHandle,16000,0.5,0.4f,2,20);
	float scores[4];
	recordHandle = alsaInit("plughw:2,0",1,12,16000,0);
	alsaStart(recordHandle);	
	char *buffer = (char *)calloc(2048,12);
	float *aec = (float *)calloc(2048,4);
	float *input = (float *)calloc(1024 * 12,4);
	short *output = (short *)calloc(1024,2);
	int doadir[4];	
//	start_speech_recognition(handleUSC);
	static int flag_speeked = 0;	

	int cloudServer = connectServer("192.168.1.116",8088);
	struct get_content getParams = {0};
	create_readFunc(cloudServer,&getParams,func_result,resOut);
	struct post_content sendData = {0};
	writeRecStart(cloudServer);
	while(1)
	{
		alsaRead(recordHandle,buffer,2048 * 12);
		micShortToFloat((short *)buffer,input,aec,9,1024);
		voiceProcess(voiceHandle,input,output,aec,doadir);
		vadNumber = huwenVadFeedData(&vadHandle,(char *)output,2048,scores);
		if(vadNumber ==1)
		{
			count += 2048;
//			automatic_speech_recognition(handleUSC,(char *)output,2048,&pBuf);
			sendData.contType = 1;
			sendData.contLength= 2048;
			memcpy(sendData.buff,(char *)output,2048);
			send(cloudServer,&sendData,sizeof(struct post_content),0);
			memset(&sendData,0,sizeof(struct post_content));
		}
		if(vadNumber == 2)
		{
//			stop_speech_recognition(handleUSC,&pBuf,func_result,resOut);
			writeRecStop(cloudServer);
			memset(buffer,0,2048 *12);
			memset(aec,0,2048*4);
			memset(input,0,1024*12*4);
			memset(output,0,1024*2);
			memset(pBuf,0,1024*10);
//			sleep(1);
			writeRecStart(cloudServer);
			
//			start_speech_recognition(handleUSC);
		}
	}

#endif
}

