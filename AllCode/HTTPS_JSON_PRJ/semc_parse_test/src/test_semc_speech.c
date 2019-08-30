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
#include "alsaInterface.h"
#include "huwenvadInterface.h"
#include "voiceInterface.h"
#include "wakeupInterface.h"

SemcParseHandle  semcParseHandTemp;
char *smdRlt;


void print_func(char * resBuf)
{
	printf("resBuf	%s\n",resBuf);											/****************** 识别结果 **************/
	semcParseResponse(resBuf,semcParseHandTemp,&smdRlt); 					/****************** 送入语义 **************/
	printf("semcPar %s\n",smdRlt);                              			/****************** 语义结果 *************/
}

extern void rcdSpeechRecognition(void (*func_result)(char*),int noSpeekTm,int maxSpeekTm)
{
	int doaData = 0,vadNumber = -1,wakeRetVal = -1,i = 0,countSc = 0;
	float scores[4];
	time_t preSeconds,curSeconds,vadOverSeconds;
	char *buffer = (char *)malloc(2048 *12);
	short *output = (short *)calloc(1024,2);
	
	HwSoundHandle uniSdHandle = hwSoundHandleInit("210.22.153.226",8999,func_result);	/************** 初始化识别句柄，默认，互问服务器IP 210.22.153.226 */
																														/***		 ，端口 8999 *********/
	
	HuwenVADHandle	vadHandle;											
	huwenVadInit(&vadHandle,16000,0.5,0.5,2,30);										/******************* 初始化 vad句柄 *******************/
	
	VoiceHandle voiceHandle;
	voiceInit(&voiceHandle);															/******************* 初始化前期处理句柄 ****************/
	int mpo_value = 12;																	
	float beam_value = 3.0;
	voiceSetParams(voiceHandle,"increasedB",&mpo_value);
	voiceSetParams(voiceHandle,"beamScale",&beam_value);

	WakeupHandle wakeHandle;															/******************* 初始化唤醒句柄 ********************/
	wakeupSingleInit(&wakeHandle,"./wakeupFile/JSnnet_1440x128x3x3_0328.txt.xiaozhi"
						,"./wakeupFile/JS_wakeup.config");
							
	
	AlsaHandle recordHandle;															/******************* 初始化录音句柄 *********************/
	recordHandle = alsaInit("plughw:1,0",1,12,16000,0);
	alsaStart(recordHandle);	
	
		

	wakeupSingleStart(wakeHandle);														/******************** 启动唤醒 ***************************/
	
	while(1)
	{
		alsaRead(recordHandle,buffer,2048 * 12);										
		doaData = voiceProcessContainPreparation(voiceHandle,buffer,output);			/* 传入12路录音流，出来单路的录音流1024个short点，即2048个字节 **/
		wakeRetVal = wakeupSingleDecode(wakeHandle,(char *)output,2048);				/******************** 检测唤醒 **************************/
		if(wakeRetVal > 0)
		{
		printf("aready WakeUped	^^^^^^^^^^^^^^^^^^^^^^^^^@@@@@@@@\n");
		printf("-----\n");
			wakeupSingleStop(wakeHandle);												/******************** 停止唤醒 **************************/
			hwSoundRecgStart(uniSdHandle);												/******************** 开启识别 **************************/
			preSeconds = time((time_t *)NULL); 
			countSc = 0;
			while(1)
			{
				alsaRead(recordHandle,buffer,2048 * 12);
				doaData = voiceProcessContainPreparation(voiceHandle,buffer,output);		
				if(countSc < 15)
				{
					hwSoundFeedData(uniSdHandle,(char *)output,2048);					/******** 传入唤醒后识别数据15次，大约1秒左右************/
					countSc++;
				}else{
					vadNumber = huwenVadFeedData(&vadHandle,(char*)output,2048,scores);	/******************* 大约1秒钟以后开启VAD检测 ***********/
					if(vadNumber == 1)
					{
						vadOverSeconds = time((time_t *)NULL);
						if(vadOverSeconds > preSeconds + maxSpeekTm)					/******** 检测到连续说话，VAD不结束，最多送入识别不能够超过的时长****/
						{
							break;														/*******  假如检测到超过时长，则结束本次识别************************/
						}
						hwSoundFeedData(uniSdHandle,(char *)output,2048);				/******** 将VAD检测到的音频流送去到识别 ****************************/ 
					}
					else if(vadNumber == 2)
					{
						break;															/**************** 说话结束，结束本次识别 ***************************/
					}
					else
					{	
						curSeconds = time((time_t *)NULL);							
						if(curSeconds > preSeconds + noSpeekTm)							/*************** 连续一定时间没有检测到语音输入，结束本次识别 ******/
						{
							break;
						}
					}
				}
			}
			hwSoundRecgStop(uniSdHandle);												/*************** 发送识别结束信号 *******************/
			wakeupSingleStart(wakeHandle);												/*************** 启动下一次唤醒 ********************/
		}
	}
}


int main()
{
	smdRlt = (char *)malloc(1024);
	semcParseHandTemp = semcParseHandInit();
	rcdSpeechRecognition(print_func,3,8);
}



