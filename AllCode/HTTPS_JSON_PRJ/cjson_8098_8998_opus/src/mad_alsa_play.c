#include <stdio.h>
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
#include "mad.h"
#include "baseModule.h"
#include "madAlsaPlay.h"

void fflushCommonBuffer(char **cData,int *cLength,int ffFlag)
{
	char cBufLeng[6] = "";
	int i = 0;
	char creadLength[5] = "";
	if(ffFlag == 0)
	{	
		sprintf(cBufLeng,"%d",*cLength);
		memcpy(commonBuff+1,cBufLeng,4);
		memcpy(commonBuff+5,*cData,*cLength);
		commonBuff[0] = 'Y';
	}else{
		strncpy(cBufLeng,commonBuff,5);
		cBufLeng[5] = '\0';
		for(i=0;i<5;i++)
			creadLength[i] = cBufLeng[i+1];
		*cLength = atoi(creadLength);
		memcpy(*cData,commonBuff+5,*cLength);
		memset(commonBuff,0,1024*8);
		commonBuff[0] = 'N';
	}
}		
	


void pourArryStream(char *arryPre,char *arryAft,int *ptrInt)
{
	int arryTemp = *ptrInt;
	int arryDeep  = arryTemp;
	
	while(arryTemp)
	{
		memcpy(arryAft+(arryDeep-arryTemp),
			arryPre+(arryTemp-1),1);
		arryTemp--;
	}	
}

void streamPushQueqe(madAlsaPlayHandle madAlsaHd,char *stmIn,int length)
{	
	MadDecAlsaBuffer * madDecHandle = (MadDecAlsaBuffer *)madAlsaHd;
	if(madDecHandle->stmArryA == NULL)
	{
		pthread_mutex_lock(&(madDecHandle->MUTEX_Stm));
			memcpy(madDecHandle->stmArryA,stmIn,length);
			madDecHandle->IntDeep = length;
		pthread_mutex_unlock(&(madDecHandle->MUTEX_Stm));
	}else{
		if(madDecHandle->IntDeep > MAX_STREAM_BUFFER)
		{
			BASE_ERROR_LOG(ErrLogHandle,"%s","can`t input, ptrInt Error ,stream  over size\n");
		}else if(((madDecHandle->IntDeep )+ length) > MAX_STREAM_BUFFER){
				while(((madDecHandle->IntDeep )+ length) > MAX_STREAM_BUFFER)
				{
					usleep(100);
				}
			pthread_mutex_lock(&(madDecHandle->MUTEX_Stm));
				memcpy(madDecHandle->stmArryA+(madDecHandle->IntDeep),stmIn,length);
				madDecHandle->IntDeep = madDecHandle->IntDeep + length;
			pthread_mutex_unlock(&(madDecHandle->MUTEX_Stm));
		}else{
			pthread_mutex_lock(&(madDecHandle->MUTEX_Stm));
				memcpy(madDecHandle->stmArryA+(madDecHandle->IntDeep),stmIn,length);
				madDecHandle->IntDeep = madDecHandle->IntDeep + length;
			pthread_mutex_unlock(&(madDecHandle->MUTEX_Stm));
		}
	}
}


int streamPullQueqe(madAlsaPlayHandle madAlsaHd,char *stmOut,int *pulSize)	
{
	MadDecAlsaBuffer * madDecHandle = (MadDecAlsaBuffer *)madAlsaHd;
	if(madDecHandle->IntDeep == 0)
	{	
		while((madDecHandle->IntDeep) == 0)
		{
			usleep(50);
		}
	}
	if((madDecHandle->IntDeep) <= PULL_ONCE_SIZE)
	{
		pthread_mutex_lock(&(madDecHandle->MUTEX_Stm));
			memcpy(stmOut,madDecHandle->stmArryA,madDecHandle->IntDeep);
			memset(madDecHandle->stmArryA,0,MAX_STREAM_BUFFER);
			*pulSize = madDecHandle->IntDeep;
			madDecHandle->IntDeep = 0;
		pthread_mutex_unlock(&(madDecHandle->MUTEX_Stm));
	}else{
		pthread_mutex_lock(&(madDecHandle->MUTEX_Stm));
			memcpy(stmOut,madDecHandle->stmArryA,PULL_ONCE_SIZE);
			memset(madDecHandle->stmArryB,0,MAX_STREAM_BUFFER);
			madDecHandle->IntDeep = madDecHandle->IntDeep - PULL_ONCE_SIZE;
			*pulSize = PULL_ONCE_SIZE;
			pourArryStream(madDecHandle->stmArryA+PULL_ONCE_SIZE,madDecHandle->stmArryB,&(madDecHandle->IntDeep));
			memset(madDecHandle->stmArryA,0,MAX_STREAM_BUFFER);
			pourArryStream(madDecHandle->stmArryB,madDecHandle->stmArryA,&(madDecHandle->IntDeep));
//			memcpy(madDecHandle->stmArryA,madDecHandle->stmArryA+PULL_ONCE_SIZE,madDecHandle->IntDeep);
//			memset(madDecHandle->stmArryA+madDecHandle->IntDeep,0,
//									(MAX_STREAM_BUFFER-madDecHandle->IntDeep));
		pthread_mutex_unlock(&(madDecHandle->MUTEX_Stm));
	}
	return 0;
}


void  set_mixer(snd_mixer_t **mixer,snd_mixer_elem_t **element)
{
	snd_mixer_open(mixer, 0);
	snd_mixer_attach(*mixer, "default");
	snd_mixer_selem_register(*mixer, NULL, NULL);
	snd_mixer_load(*mixer);
	*element = snd_mixer_first_elem(*mixer);	
	*element = snd_mixer_elem_next(*element);	
	long long int alsa_min_vol, alsa_max_vol;
	snd_mixer_selem_get_playback_volume_range(*element,&alsa_min_vol,&alsa_max_vol);
	snd_mixer_selem_set_playback_volume_range(*element, 0, 100);
}
int curVoiceValue(snd_mixer_t *mixer,snd_mixer_elem_t *element)
{
	long long int leftVal = 0, rightVal = 0;	
	snd_mixer_handle_events(mixer);
	snd_mixer_selem_get_playback_volume(element,
				SND_MIXER_SCHN_FRONT_LEFT,&leftVal);
	snd_mixer_selem_get_playback_volume(element,
				SND_MIXER_SCHN_FRONT_RIGHT,&rightVal);
	
	return ((leftVal+rightVal) >> 1);
}
void changeVoiceValue(int value,snd_mixer_elem_t *element)
{
	snd_mixer_selem_set_playback_volume(element,
					SND_MIXER_SCHN_FRONT_LEFT,value);

	snd_mixer_selem_set_playback_volume(element,
					SND_MIXER_SCHN_FRONT_RIGHT,value);
}
void destroyMixter(snd_mixer_t *mixer)
{	
	snd_mixer_close(mixer);
}


static void single2Double(char *pData, int nSize)  
{  
   	unsigned short szBuf[8192];    
   	unsigned short *pst = (unsigned short*)pData;  
    memset(szBuf, 0, sizeof(szBuf));  
    memcpy(szBuf, pData, nSize);  
    for (int i = 0; i < nSize; i++)  
    {  
        if (i % 2 == 0)   
        {   
            pst[i] = szBuf[i / 2];  
        }  
        else   
        {  
             pst[i] = pst[i - 1];   
        }   
   }    
} 
 
static enum mad_flow input(void *data,struct mad_stream *stream)
{
	struct buffer *buffer = data;
	sem_wait(&SEM_PLAY_READED);
	MadDecAlsaBuffer * madDecHandle = (MadDecAlsaBuffer *)buffer->madAlsaHd;

	fflushCommonBuffer((char **)&madDecHandle->chPlayStart,(int*)&buffer->length,1);
	sem_post(&SEM_PLAY_WRITED);
	memset(madDecHandle->chPlayNow,0,1024*8);
	int leftSize = stream->bufend - stream->next_frame;
			
	memcpy(madDecHandle->chPlayNow,madDecHandle->chPlayPre+buffer->lengPre-leftSize,leftSize);
	memcpy(madDecHandle->chPlayNow+leftSize,madDecHandle->chPlayStart,buffer->length);
	memcpy(madDecHandle->chPlayPre,madDecHandle->chPlayStart,buffer->length);
	buffer->lengPre = buffer->length;
	mad_stream_buffer(stream, madDecHandle->chPlayNow, buffer->length +leftSize);
	return MAD_FLOW_CONTINUE;
}

static inline signed int scale(mad_fixed_t sample)
{
	sample += (1L << (MAD_F_FRACBITS - 16));
  	if (sample >= MAD_F_ONE)
   		sample = MAD_F_ONE - 1;
  	else if (sample < -MAD_F_ONE)
    	sample = -MAD_F_ONE;
  	return sample >> (MAD_F_FRACBITS + 1 - 16);
}

static enum mad_flow output(void *data,struct mad_header const *header,struct mad_pcm *pcm)
{
	struct play_params paramPlay={0};
	struct buffer *buffer = data;
	int curValue = 0;
	unsigned int nchannels, nsamples,n;
	mad_fixed_t const *left_ch, *right_ch;

	unsigned int pcm_rate = pcm->samplerate;
  /* pcm->samplerate contains the sampling frequency */

  	unsigned short pcm_channel = nchannels = pcm->channels;
  	n=nsamples  = pcm->length;

	
  	left_ch   = pcm->samples[0];
  	right_ch  = pcm->samples[1];

  	unsigned char Output[6912], *OutputPtr;  
  	int fmt, wrote, speed, exact_rate, err, dir;  
   	OutputPtr = Output;  
  
  	while (nsamples--) 
   	{
    	signed int sample;

    /* output sample(s) in 16-bit signed little-endian PCM */
    
    	sample = scale(*left_ch++);
   
    	*(OutputPtr++) = sample >> 0;  
    	*(OutputPtr++) = sample >> 8;  
    	if (nchannels == 2)  
        {  
        	sample = scale (*right_ch++);  
            *(OutputPtr++) = sample >> 0;  
            *(OutputPtr++) = sample >> 8;  
        }  
  	}
    OutputPtr = Output;
	
	paramPlay.handleMixer = &buffer->mixer;
	paramPlay.handleElement = &buffer->element;

	paramPlay.madAlsaHandle = buffer->madAlsaHd;
	paramPlay.srLength = n * pcm->channels * 2;
	paramPlay.outPtr = OutputPtr;
	paramPlay.samRate = pcm_rate;
	paramPlay.chnNum = pcm_channel;
	paramPlay.preTTSFlag = buffer->lengPre;
	buffer->playMP3(&paramPlay);
	
	OutputPtr = Output;     
 	 return MAD_FLOW_CONTINUE;
}

static enum mad_flow error(void *data,struct mad_stream *stream,struct mad_frame *frame)
{
	struct buffer *buffer = data;
#if 0
	printf("this is mad_flow error\n");
	fprintf(stderr, "decoding error 0x%04x (%s) at byte offset %ld\n",
    	stream->error, mad_stream_errorstr(stream),
     	stream->this_frame - buffer->start);
#endif 
  	return MAD_FLOW_CONTINUE;
}




void playMusic(struct play_params* param)
{
	
	MadDecAlsaBuffer * madDecHandle = (MadDecAlsaBuffer *)param->madAlsaHandle;
	if(THROW_FRAME_LEFT)
	{
		pthread_mutex_lock(&(madDecHandle->MUTEX_AlsaPlay));
			THROW_FRAME_LEFT = 0;
		pthread_mutex_unlock(&(madDecHandle->MUTEX_AlsaPlay));
		return;
	}
	switch(CONTROL_STREAM_FLAG)
	{
		case PAUSE_RESUME_PAUSE:
				while(CONTROL_STREAM_FLAG == PAUSE_RESUME_PAUSE)
					usleep(100);
				if(CONTROL_STREAM_FLAG == PAUSE_RESUME_STOP)
				{
					char bufNull[4096] = "";
					alsaSend(madDecHandle->AlsaPlayHandle,bufNull,4096);
					return;
				}
				break;
		case PAUSE_RESUME_STOP:
				{
					char bufNull[4096] = "";
					alsaSend(madDecHandle->AlsaPlayHandle,bufNull,4096);
					return;
				}
#if 0			
		case DOWN_VOICE:		
				set_mixer(param->handleMixer,param->handleElement);
				curValue = curVoiceValue(*(param->handleMixer),*(param->handleElement));
				changeVoiceValue(curValue - 5,*(param->handleElement));
				destroyMixter(*(param->handleMixer));
				*(param->pauseFlag) = 0;
				break;
		case UPPER_VOICE:
				set_mixer(param->handleMixer,param->handleElement);
				curValue = curVoiceValue(*(param->handleMixer),*(param->handleElement));
				changeVoiceValue(curValue + 5,*(param->handleElement));
				destroyMixter(*(param->handleMixer));
				*(param->pauseFlag)  = 0;
				break;
#endif
		default: 
				break;
	}
	int inputLen = 0,ouputLen = 0, resaInLen = 0;
	if(param->samRate == 44100)
	{
		alsaSend(madDecHandle->AlsaPlayHandle,(char*)(param->outPtr),param->srLength);
	}else{
		memset(madDecHandle->int16InBuf,0,1024*4);
		memcpy(madDecHandle->int16InBuf,param->outPtr,param->srLength);	
		memset(madDecHandle->int16OutBuf,0,1024*16);
		resaInLen = param->srLength;	
		ouputLen = resaInLen *2;
		inputLen = resaInLen / 2;
		switch(param->samRate)
		{
			case 16000:
						speex_resampler_process_int(madDecHandle->resampler16, 0, madDecHandle->int16InBuf,
							&inputLen,madDecHandle->int16OutBuf, &ouputLen); 
						break;

			case 22050:
						speex_resampler_process_int(madDecHandle->resampler225, 0, madDecHandle->int16InBuf,
							&inputLen,madDecHandle->int16OutBuf, &ouputLen); 
						break;

			case 48000:
						speex_resampler_process_int(madDecHandle->resampler48, 0, madDecHandle->int16InBuf,
							&inputLen,madDecHandle->int16OutBuf, &ouputLen); 
						break;

		}	
		
		memcpy(madDecHandle->outPlayBuf,madDecHandle->int16OutBuf,ouputLen *2);
		if(param->chnNum == 1)
		{
			single2Double(madDecHandle->outPlayBuf,ouputLen *4);	
			ouputLen = ouputLen * 2;
		}
		alsaSend(madDecHandle->AlsaPlayHandle,madDecHandle->outPlayBuf,ouputLen*2);
	}

}

		

void *cmdPlaying(void *arg)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,0);
	static struct buffer  buffStream;	
	static struct mad_decoder decoder;	
	buffStream.madAlsaHd = (madAlsaPlayHandle)arg;

	buffStream.length = 0;
	buffStream.lengPre = 0;
	buffStream.mixer = NULL;
	buffStream.element = NULL;
	buffStream.playMP3 = playMusic;	

	mad_decoder_init(&decoder,&buffStream,input,0,0,output,error,0);
   	int result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
	
}


int decPlayingThread(madAlsaPlayHandle madAlsaHd)
{
	pthread_t playThread;
    pthread_attr_t  attrControl;

	pthread_attr_init(&attrControl);
	pthread_attr_setdetachstate(&attrControl,PTHREAD_CREATE_DETACHED);
	if(pthread_create(&playThread,&attrControl,cmdPlaying,(void*)madAlsaHd)!=0)
		PERROR(-1,"\npthread_create error");		
	
}


extern madAlsaPlayHandle  madDecAlsaPlayInit(char *alsaPlayDev)
{
	MadDecAlsaBuffer * madDecHandle = (MadDecAlsaBuffer *)malloc(sizeof(MadDecAlsaBuffer));
	madDecHandle->int16OutBuf = (int16_t *)malloc(1024 *16);
	madDecHandle->outPlayBuf = (char *)malloc(1024 * 32);
	madDecHandle->resampler16 = speex_resampler_init(1, 16000, 44100, 10, NULL);
	madDecHandle->resampler225 =  speex_resampler_init(1, 22050, 44100, 10, NULL);
	madDecHandle->resampler48 = speex_resampler_init(2, 48000, 44100, 10, NULL);
	speex_resampler_skip_zeros(madDecHandle->resampler16);
	speex_resampler_skip_zeros(madDecHandle->resampler225);
	speex_resampler_skip_zeros(madDecHandle->resampler48);
	madDecHandle->AlsaPlayHandle = alsaInit(alsaPlayDev,0,2,44100,0);
	
	alsaStart(madDecHandle->AlsaPlayHandle);

	madDecHandle->chPlayPre = (char *)malloc(1024* 4);
	madDecHandle->chPlayStart = (char *)malloc(1024* 4);
	madDecHandle->chPlayNow = (char *)malloc(1024* 8);
	madDecHandle->stmArryA = (char *)malloc(MAX_STREAM_BUFFER);
	madDecHandle->stmArryB = (char *)malloc(MAX_STREAM_BUFFER);
	madDecHandle->IntDeep = 0;

	madDecHandle->httpMp3Buff = (char *)malloc(1024* 4);
	
	pthread_mutex_init(&(madDecHandle->MUTEX_AlsaPlay),NULL);
	pthread_mutex_init(&(madDecHandle->MUTEX_Stm),NULL);
	pthread_mutex_init(&(madDecHandle->MUTEX_HttpDown),NULL);
	pthread_mutex_init(&MUTEX_DwStm,NULL);
	sem_init(&PTHREAD_CHANGE,0,0);
	sem_init(&SEM_PLAY_READED,0,0);
	sem_init(&SEM_PLAY_WRITED,0,0);
	sem_post(&PTHREAD_CHANGE);	
	
	decPlayingThread((madAlsaPlayHandle)madDecHandle);
	return (madAlsaPlayHandle)madDecHandle;
}


void dataBoradPlay(char *httpName,char *postMp3Http,madAlsaPlayHandle madAlsaHd) 
{	
	MadDecAlsaBuffer *madDecHandle = (MadDecAlsaBuffer *)madAlsaHd;
	char postMP3Buff[512] = "";
	char httpBuff[50] = "";
	strcpy(httpBuff,httpName);
	strcpy(postMP3Buff,postMp3Http);
	
	pthread_mutex_lock(&MUTEX_DwStm);
		CONTROL_STREAM_FLAG = PAUSE_RESUME_STOP;
	pthread_mutex_unlock(&MUTEX_DwStm);
	if(SOCKET_FOR_CLOSE > 0)
	{
		close(SOCKET_FOR_CLOSE);
		pthread_mutex_lock(&MUTEX_DwStm);
			SOCKET_FOR_CLOSE = 0;
		pthread_mutex_unlock(&MUTEX_DwStm);
	}
	BASE_INFO_LOG(InfoLogHandle,"%s","data^^^^^^^^	sem_wait_pthread_change\n");
	sem_wait(&PTHREAD_CHANGE);
	pthread_mutex_lock(&MUTEX_DwStm);
		memset(commonBuff,0,1024*8);
		commonBuff[0] = 'N';
		CONTROL_STREAM_FLAG = PAUSE_RESUME_RESUME;
		THROW_FRAME_LEFT = 1;
	pthread_mutex_unlock(&MUTEX_DwStm);
	BASE_INFO_LOG(InfoLogHandle,"%s","dataBoradPlay^^^^^^^^^^^^_over_sem_\n");
	int ttsSockfd = 0,length = -1,i = 0,index = 0,isfind = 0,retVal = 0;
	struct hostent *ttsHost;
	ttsHost = gethostbyname(httpBuff);	
	char * ipAddr = inet_ntoa(*(struct in_addr*)ttsHost->h_addr_list[0]);
	ttsSockfd = socketCreate(ipAddr,80);	
	pthread_mutex_lock(&MUTEX_DwStm);
		SOCKET_FOR_CLOSE = ttsSockfd;
	pthread_mutex_unlock(&MUTEX_DwStm);	
	retVal = write(ttsSockfd,postMP3Buff,strlen(postMP3Buff));
	char url[1024];
	memset(url,'\0',1024);
	char line[2049];
	char * palyMediaBuff = madDecHandle->httpMp3Buff;
	memset(palyMediaBuff,0,4094);
	int flagContLen = 0;
	int sumBufferLength = 0;
	while ((length = recv(ttsSockfd,url,1,0)) == 1) {
		if (i < 4) {
			if (flagContLen == 0) {
				line[index++] = url[0];
				if (url[0] == ':' && isfind == 0) {
					line[index - 1] = '\0';
					if (memcmp(line,"Content-Length",strlen("Content-Length")) == 0) {
						isfind = 1;
						index = 0;
					}
				}
			}
			if (url[0] == '\r' || url[0] == '\n') {
				if (isfind == 1 && flagContLen == 0) {
					line[index] = '\0';
					sumBufferLength = atoi(line);
					flagContLen = 1;
					if(sumBufferLength < 1000)
					{
						BASE_ERROR_LOG(ErrLogHandle,"%s%d","getting mp3Buffer error __sumBufferLength: ",sumBufferLength);
						sem_post(&PTHREAD_CHANGE);
						return;
					}
				}
				index = 0;
				i++;
			} else {
				i = 0;
			}
		} else {
			
			length = 0;
			retVal = recv(ttsSockfd,palyMediaBuff,BASE_BUF*4,0);
			if(retVal <= 0)
			{
				BASE_ERROR_LOG(ErrLogHandle,"%s","recv stream Failed return~");
				return;
			}
			fflushCommonBuffer(&palyMediaBuff,&retVal,0);
			sem_post(&SEM_PLAY_READED);
			length += retVal;
			while(length+1 < sumBufferLength)
			{
				memset(palyMediaBuff,0,4094);
				retVal = recv(ttsSockfd,palyMediaBuff,BASE_BUF*4,0);
				if(retVal <= 0)
				{
					BASE_INFO_LOG(InfoLogHandle,"%s","being closed  by others ^^^^\n");
					break;
				}
				sem_wait(&SEM_PLAY_WRITED);
				fflushCommonBuffer(&palyMediaBuff,&retVal,0);
				sem_post(&SEM_PLAY_READED);
				length += retVal;
			}
			retVal = 0;
			sem_wait(&SEM_PLAY_WRITED);
			fflushCommonBuffer(&palyMediaBuff,&retVal,0);
			sem_post(&SEM_PLAY_READED);	
			BASE_INFO_LOG(InfoLogHandle,"%s","sem_wait paly Writed &&&&&&&&&&&&&&&&&&&&&&&&&&^\n");
			sem_wait(&SEM_PLAY_WRITED);	
			sem_post(&PTHREAD_CHANGE);
			if(SOCKET_FOR_CLOSE > 1)
			{
				close(SOCKET_FOR_CLOSE);
				pthread_mutex_lock(&MUTEX_DwStm);
					SOCKET_FOR_CLOSE = 0;
				pthread_mutex_unlock(&MUTEX_DwStm);		
			}
			return;
		}
	}
}
#if 0
 
void dataBoradCast(char *httpName,char * GET_HOST,char * GET_HTTP,char *text,madAlsaPlayHandle madAlsaHd) 
{
	MadDecAlsaBuffer *madDecHandle = (MadDecAlsaBuffer *)madAlsaHd;
	int i = 0,j = 0, lengthCmd = 0;
	char cmdFilter[4096];
	lengthCmd = strlen(text);
	for(i = 0;i < lengthCmd;i++)
	{
		if(text[i] != ' ')
		{
			cmdFilter[j++] = text[i];
		}
	}
	cmdFilter[j] = '\0';
	pthread_mutex_lock(&MUTEX_DwStm);
		CONTROL_STREAM_FLAG = PAUSE_RESUME_STOP;
	pthread_mutex_unlock(&MUTEX_DwStm);
	if(SOCKET_FOR_CLOSE > 0)
	{
		close(SOCKET_FOR_CLOSE);
		pthread_mutex_lock(&MUTEX_DwStm);
			SOCKET_FOR_CLOSE = 0;
		pthread_mutex_unlock(&MUTEX_DwStm);
	}
	BASE_INFO_LOG(InfoLogHandle,"%s","dataBoradCast ^^^^^^^^^^^##########	PTHREAD_CHANGE_WAIT\n");

	sem_wait(&PTHREAD_CHANGE);
	pthread_mutex_lock(&MUTEX_DwStm);
		memset(commonBuff,0,1024*8);
		commonBuff[0] = 'N';
		CONTROL_STREAM_FLAG = PAUSE_RESUME_RESUME;
	pthread_mutex_unlock(&MUTEX_DwStm);
	int ttsSockfd = 0,length = -1,index = 0,isfind = 0,retVal = 0;
	i = 0;
	struct hostent *ttsHost;
	ttsHost = gethostbyname(httpName);	
//		printf("address type: %s \n",(ttsHost->h_addrtype==AF_INET)?"AF_INET":"AF_INET6");
	char * ipAddr = inet_ntoa(*(struct in_addr*)ttsHost->h_addr_list[0]);
	ttsSockfd = socketCreate(ipAddr,80);
	pthread_mutex_lock(&MUTEX_DwStm);
		SOCKET_FOR_CLOSE = ttsSockfd;
	pthread_mutex_unlock(&MUTEX_DwStm);
	char ttsBuf[4096] = "";
	sprintf(ttsBuf,"%s%s%s%s%s%s%s",
			GET_HTTP,
			cmdFilter,
	 		" HTTP/1.1\r\n",
	 		GET_HOST,
	 		"Connection: close\r\n",
	 		"Accept: */*\r\n",
	 		"\r\n"
			 );
	retVal = write(ttsSockfd,ttsBuf,strlen(ttsBuf));
	if(!contentIsUtf8(cmdFilter))
		return;
	char *stmBuff = madDecHandle->httpMp3Buff;
	memset(stmBuff,0,4096);
	char url[1024];
	memset(url,'\0',1024);
	char line[2049];
	int buffer = 0;
	while ((length = read(ttsSockfd,url,1)) > 0) {
		if (i < 4) {
			if (buffer == 0) {
				line[index++] = url[0];
				if (url[0] == ':' && isfind == 0) {
					line[index - 1] = '\0';
					if (memcmp(line,"Content-Length",strlen("Content-Length")) == 0) {
						isfind = 1;
						index = 0;
					}
				}
			}
			if (url[0] == '\r' || url[0] == '\n') {
				if (isfind == 1 && 0 == buffer) {
					line[index] = '\0';
					buffer = 1;
				}
				index = 0;
				i++;
			} else {
				i = 0;
			}
		} else {
			length = recv(ttsSockfd,stmBuff,BASE_BUF*4,0);
				PERROR(length,"\nrecv TTs BroadCast Error\n");
			fflushCommonBuffer(&stmBuff,&length,0);
			sem_post(&SEM_PLAY_READED);
			while(1)
			{	
				memset(stmBuff,0,4096);
				length = recv(ttsSockfd,stmBuff,BASE_BUF*4,0);
				if(length < 1)
					break;
				sem_wait(&SEM_PLAY_WRITED);
				fflushCommonBuffer(&stmBuff,&length,0);
				sem_post(&SEM_PLAY_READED);
			}
			break;
		}
	}
	
	
#if 0	
	length = recv(ttsSockfd,stmBuff,BASE_BUF*4,0);
		PERROR(length,"\nrecv TTs BroadCast Error\n");
	fflushCommonBuffer(&stmBuff,&length,0);
	printf("%s\n",stmBuff);
	sem_post(&SEM_PLAY_READED);
	while(1)
	{	
		memset(stmBuff,0,4096);
		length = recv(ttsSockfd,stmBuff,BASE_BUF*4,0);
		if(length < 1)
			break;
		sem_wait(&SEM_PLAY_WRITED);
		fflushCommonBuffer(&stmBuff,&length,0);
		sem_post(&SEM_PLAY_READED);
	}
#endif
	if(SOCKET_FOR_CLOSE > 1)
	{
		close(SOCKET_FOR_CLOSE);
		pthread_mutex_lock(&MUTEX_DwStm);
			SOCKET_FOR_CLOSE = 0;
		pthread_mutex_unlock(&MUTEX_DwStm);		
	}
	length = 0;
	sem_wait(&SEM_PLAY_WRITED);
	fflushCommonBuffer(&stmBuff,&length,0);
	sem_post(&SEM_PLAY_READED);

	BASE_INFO_LOG(InfoLogHandle,"%s","dataBoradCast ^^^^end  ####### _WAIT	writed\n");
	sem_wait(&SEM_PLAY_WRITED);
	sem_post(&PTHREAD_CHANGE);
}

#endif
void dataBoradCast(char *httpName,char * GET_HOST,char * GET_HTTP,char *text,madAlsaPlayHandle madAlsaHd) 
{
	MadDecAlsaBuffer *madDecHandle = (MadDecAlsaBuffer *)madAlsaHd;
	int i = 0,j = 0, lengthCmd = 0;
	char cmdFilter[4096];
	lengthCmd = strlen(text);
	for(i = 0;i < lengthCmd;i++)
	{
		if(text[i] != ' ')
		{
			cmdFilter[j++] = text[i];
		}
	}
	cmdFilter[j] = '\0';
	pthread_mutex_lock(&MUTEX_DwStm);
		CONTROL_STREAM_FLAG = PAUSE_RESUME_STOP;
	pthread_mutex_unlock(&MUTEX_DwStm);
	if(SOCKET_FOR_CLOSE > 0)
	{
		close(SOCKET_FOR_CLOSE);
		pthread_mutex_lock(&MUTEX_DwStm);
			SOCKET_FOR_CLOSE = 0;
		pthread_mutex_unlock(&MUTEX_DwStm);
	}
	BASE_INFO_LOG(InfoLogHandle,"%s","dataBoradCast ^^^^^^^^^^^##########	PTHREAD_CHANGE_WAIT\n");

	sem_wait(&PTHREAD_CHANGE);
	pthread_mutex_lock(&MUTEX_DwStm);
		CONTROL_STREAM_FLAG = PAUSE_RESUME_RESUME;
		THROW_FRAME_LEFT = 1;
	pthread_mutex_unlock(&MUTEX_DwStm);
	int ttsSockfd = 0,length = -1,index = 0,isfind = 0,retVal = 0;
	i = 0;
	struct hostent *ttsHost;
	ttsHost = gethostbyname(httpName);	
//		printf("address type: %s \n",(ttsHost->h_addrtype==AF_INET)?"AF_INET":"AF_INET6");
	char * ipAddr = inet_ntoa(*(struct in_addr*)ttsHost->h_addr_list[0]);
	ttsSockfd = socketCreate(ipAddr,80);
	pthread_mutex_lock(&MUTEX_DwStm);
		SOCKET_FOR_CLOSE = ttsSockfd;
	pthread_mutex_unlock(&MUTEX_DwStm);
	char ttsBuf[4096] = "";
	sprintf(ttsBuf,"%s%s%s%s%s%s%s",
			GET_HTTP,
			cmdFilter,
	 		" HTTP/1.1\r\n",
	 		GET_HOST,
	 		"Connection: close\r\n",
	 		"Accept: */*\r\n",
	 		"\r\n"
			 );
	retVal = write(ttsSockfd,ttsBuf,strlen(ttsBuf));
	if(!contentIsUtf8(cmdFilter))
		return;

	char *stmBuff = madDecHandle->httpMp3Buff;
	memset(stmBuff,0,4096);
	memset(commonBuff,0,1024*8);
	commonBuff[0] = 'N';
	length = recv(ttsSockfd,stmBuff,BASE_BUF*4,0);
		PERROR(length,"\nrecv TTs BroadCast Error\n");

	fflushCommonBuffer(&stmBuff,&length,0);
	sem_post(&SEM_PLAY_READED);
	while(1)
	{	
		memset(stmBuff,0,4096);
		length = recv(ttsSockfd,stmBuff,BASE_BUF*4,0);
		if(length < 1)
			break;
		sem_wait(&SEM_PLAY_WRITED);
		fflushCommonBuffer(&stmBuff,&length,0);
		sem_post(&SEM_PLAY_READED);
	}
	if(SOCKET_FOR_CLOSE > 1)
	{
		close(SOCKET_FOR_CLOSE);
		pthread_mutex_lock(&MUTEX_DwStm);
			SOCKET_FOR_CLOSE = 0;
		pthread_mutex_unlock(&MUTEX_DwStm);		
	}
	length = 0;
	sem_wait(&SEM_PLAY_WRITED);
	fflushCommonBuffer(&stmBuff,&length,0);
	sem_post(&SEM_PLAY_READED);

	BASE_INFO_LOG(InfoLogHandle,"%s","dataBoradCast ^^^^end  ####### _WAIT	writed\n");
	sem_wait(&SEM_PLAY_WRITED);
	sem_post(&PTHREAD_CHANGE);
}



