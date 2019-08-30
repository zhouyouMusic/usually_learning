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
#include "baseModule.h"
#include "semcParse.h"
#include "madAlsaPlay.h"
#include "huwenvadInterface.h"
#include "voiceInterface.h"
#include "speechBaseModule.h"
#include "wakeupInterface.h"
#include "r16_gpio_control.h"

pthread_mutex_t		MUTEX_RcdRcg;

static void micShortToFloat(short *input,float *output,float *aec,int nMics,int length)
{
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

static void micfloatToShort(float *input,short *output,int length)
{
	int i = 0, j = 0;
	for(j = 0;j < length;j++)
	{
		output[j] = input[j] / 0.00003052f;
	}
}


extern void rcdSpeechRecognition(void (*func_result)(char*),int noSpeekTm,int maxSpeekTm)
{
	pthread_mutex_init(&MUTEX_RcdRcg,NULL);
	periControl(32,0x77771177,"0x01c2086c");
	periControl(32,0x00005C00,"0x01c2087c");
	int doaData = 0,vadNumber = -1,wakeRetVal = -1,i = 0,countSc = 0;
	float scores[4];
	time_t preSeconds,curSeconds,vadOverSeconds;
	char *buffer = (char *)malloc(2048 *12);
	short *output = (short *)calloc(1024,2);
	
	HwSoundHandle uniSdHandle = hwSoundHandleInit("192.168.1.116",8998,func_result);	
	
	HuwenVADHandle	vadHandle;
	huwenVadInit(&vadHandle,16000,0.5,0.5,2,30);

	VoiceHandle voiceHandle;
	voiceInit(&voiceHandle);
	int mpo_value = 9;
	float beam_value = 3.0;
	voiceSetParams(voiceHandle,"increasedB",&mpo_value);
	voiceSetParams(voiceHandle,"beamScale",&beam_value);


	WakeupHandle wakeHandle;
	wakeupSingleInit(&wakeHandle,"./wakeupFile/JSnnet_1440x128x3x3_0328.txt.xiaozhi"
						,"./wakeupFile/JS_wakeup.config");
			
	AlsaHandle recordHandle;
	recordHandle = alsaInit("plughw:1,0",1,12,16000,0);
	alsaStart(recordHandle);	
	
	wakeupSingleStart(wakeHandle);

//FILE * fp1 = fopen("rcd.pcm","wb");
//FILE * fp2 = fopen("out.pcm","wb");

	while(1)
	{
		alsaRead(recordHandle,buffer,2048 * 12);
		doaData = voiceProcessContainPreparation(voiceHandle,buffer,output);
		wakeRetVal = wakeupSingleDecode(wakeHandle,(char *)output,2048);
		if(wakeRetVal > 0)
		{
			wakeupSingleStop(wakeHandle);
			periControl(32,0x00005C0f,"0x01c2087c");
			BASE_INFO_LOG(InfoLogHandle,"%s","****************Aready**Wakeup\n");		
			pthread_mutex_lock(&MUTEX_RcdRcg);
				CONTROL_STREAM_FLAG = 3;
			pthread_mutex_unlock(&MUTEX_RcdRcg);
			hwSoundRecgStart(uniSdHandle);

			preSeconds = time((time_t *)NULL); 
			countSc = 0;
			while(1)
			{
				alsaRead(recordHandle,buffer,2048 * 12);
//		fwrite(buffer,2048 * 12,1,fp1);
				doaData = voiceProcessContainPreparation(voiceHandle,buffer,output);
//		fwrite(output,2048,1,fp2);
				if(countSc < 15)
				{
					hwSoundFeedData(uniSdHandle,(char *)output,2048);
					countSc++;
				}else{
					vadNumber = huwenVadFeedData(&vadHandle,(char*)output,2048,scores);
					if(vadNumber == 1)
					{
						vadOverSeconds = time((time_t *)NULL);
						if(vadOverSeconds > preSeconds + maxSpeekTm)
						{
							break;
						}
						hwSoundFeedData(uniSdHandle,(char *)output,2048);
					}
					else if(vadNumber == 2)
					{
						BASE_INFO_LOG(InfoLogHandle,"%s","vad detected ended Speek\n");
						break;	
					}
					else
					{	
						curSeconds = time((time_t *)NULL);
						if(curSeconds > preSeconds + noSpeekTm)
						{
							BASE_INFO_LOG(InfoLogHandle,"%s","three Sec No speeking End WakeUp\n");
							break;
						}
					}
				}
			}
			periControl(32,0x00005C00,"0x01c2087c");
			hwSoundRecgStop(uniSdHandle);
			BASE_INFO_LOG(InfoLogHandle,"%s","begain to start Wakeup  \n");
			wakeupSingleStart(wakeHandle);
		}
	}
}

