#include <stdio.h>
#include <stdlib.h>
#include <linux/soundcard.h>
#include <semaphore.h>
#include <pthread.h>
#include <signal.h>
#include "mad.h"
#include "alsaPlay.h"


#if 0
static void  set_mixer(snd_mixer_t **mixer,snd_mixer_elem_t **element)
{
	snd_mixer_open(mixer, 0);
	snd_mixer_attach(*mixer, "default");
	snd_mixer_selem_register(*mixer, NULL, NULL);
	snd_mixer_load(*mixer);
	*element = snd_mixer_first_elem(*mixer);	
	long alsa_min_vol, alsa_max_vol;
	snd_mixer_selem_get_playback_volume_range(*element,
	&alsa_min_vol,
	&alsa_max_vol);
	snd_mixer_selem_set_playback_volume_range(*element, 0, 100);
}
static long curVoiceValue(snd_mixer_t *mixer,snd_mixer_elem_t *element)
{
	long leftVal,rightVal;	
	snd_mixer_handle_events(mixer);
	snd_mixer_selem_get_playback_volume(element,
				SND_MIXER_SCHN_FRONT_LEFT,&leftVal);
	snd_mixer_selem_get_playback_volume(element,
				SND_MIXER_SCHN_FRONT_RIGHT,&rightVal);
	return (leftVal+rightVal) >> 1;
}
static void changeVoiceValue(int value,snd_mixer_elem_t *element)
{
	snd_mixer_selem_set_playback_volume(element,
					SND_MIXER_SCHN_FRONT_LEFT,value);

	snd_mixer_selem_set_playback_volume(element,
					SND_MIXER_SCHN_FRONT_RIGHT,value);
}
static void destroyMixter(snd_mixer_t *mixer)
{	
	snd_mixer_close(mixer);
}

static int set_pcm(snd_pcm_t** pcmHandle,snd_pcm_hw_params_t **pcmParams,int rateSet,int channelSet)
{
    int  rc = -1, dir = 0;
    int rate = rateSet;	//16000  44100            /* ?憸? 44.1KHz*/
    int format = SND_PCM_FORMAT_S16_LE; /*     ??雿 16      */
    int channels = channelSet;     // 1 or 2            /*     憯圈???2           */
    int buffer_time = 50000;// 500000;
    int period_time = 10000;///10000;
    *pcmHandle = NULL;
	*pcmHandle = NULL;
    rc = snd_pcm_open(pcmHandle, "default", SND_PCM_STREAM_PLAYBACK, 0);
		PERROR(rc,"\nopen PCM device failed:");
    	snd_pcm_hw_params_alloca(pcmParams); //??params蝏?雿?
    rc = snd_pcm_hw_params_any(*pcmHandle, *pcmParams);//???arams
    	PERROR(rc,"\nsnd_pcm_hw_params_any:");
    rc = snd_pcm_hw_params_set_access(*pcmHandle,*pcmParams, SND_PCM_ACCESS_RW_INTERLEAVED);  //???挪?格???
		PERROR(rc,"\nsed_pcm_hw_set_access:");   
    rc = snd_pcm_hw_params_set_format(*pcmHandle,*pcmParams, SND_PCM_FORMAT_S16_LE);             //霈曄蔭16雿??瑞移摨? 
    	PERROR(rc,"\nsnd_pcm_hw_params_set_format failed:");  
    rc =  snd_pcm_hw_params_set_channels(*pcmHandle,*pcmParams, channels);  //霈曄蔭憯圈?,1銵函內?ㄟ>??2銵函內蝡?憯?
		PERROR(rc,"\nsnd_pcm_hw_params_set_channels:");  
    rc = snd_pcm_hw_params_set_rate_near(*pcmHandle,*pcmParams, &rate, &dir);  //霈曄蔭>憸?
		PERROR(rc,"\nsnd_pcm_hw_params_set_rate_near:");  
	rc = snd_pcm_hw_params_set_buffer_time_near(*pcmHandle,*pcmParams,&buffer_time, &dir);
		PERROR(rc,"\nnd_pcm_buffer_errr:");  
	rc = snd_pcm_hw_params_set_period_time_near(*pcmHandle,*pcmParams,&period_time, &dir);
		PERROR(rc,"\nnd_pcm_period_errr:");  
    rc = snd_pcm_hw_params(*pcmHandle,*pcmParams);
		PERROR(rc,"\nsnd_pcm_hw_params:");     
    return 0;              
}

#endif
 
static enum mad_flow input(void *data,struct mad_stream *stream)
{
	struct buffer *buffer = data;
	if(buffer->lengPre  == 8888)
	{
		if (!buffer->length)
    		return MAD_FLOW_STOP;		
		mad_stream_buffer(stream, buffer->start, buffer->length);
		buffer->length = 0;
		return MAD_FLOW_CONTINUE;
	}
#if 1
	if (!buffer->length)
    		return MAD_FLOW_STOP;		
	buffer->gettingBuff((char **)&buffer->start,(int*)&buffer->length,1);
	memset(buffer->playing,0,1024*8);
	int leftSize = stream->bufend - stream->next_frame;
		
	memcpy(buffer->playing,buffer->pre+buffer->lengPre-leftSize,leftSize);
	memcpy(buffer->playing+leftSize,buffer->start,buffer->length);
	memcpy(buffer->pre,buffer->start,buffer->length);
	buffer->lengPre = buffer->length;
  	mad_stream_buffer(stream, buffer->playing, buffer->length +leftSize);
//	fwrite(buffer->start,buffer->length,1,hello);
#endif
//	buffer->length = 0;
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
//	printf("stream out @@@@@@@@@@@@@@@***********************	\n");
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

//	pthread_mutex_lock(&mutex);	  

	*(buffer->ENDING_FLAG)= 0;
	
//	pthread_mutex_unlock(&mutex);   

	paramPlay.handlePlay = &buffer->palyHandle;
//	paramPlay.handleMixer = &buffer->mixer;
//	paramPlay.handleElement = &buffer->element;

	paramPlay.srLength = n;
	paramPlay.outPtr = OutputPtr;
	paramPlay.pauseFlag = buffer->PAUSE_FLAG;
	paramPlay.setPcmFlag = buffer->SET_PCM_FLAG;
	paramPlay.samRate = pcm_rate;
	paramPlay.chnNum = pcm_channel;

	buffer->playMP3(&paramPlay);
	
	OutputPtr = Output;     
//	printf("under out @@@@@@@@@@@@@@@	***********************\n");	
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
	int curValue = 0;
	if(*(param->setPcmFlag))
	{	
		*(param->setPcmFlag) = 0;
		*(param->pauseFlag) = 0;
		if(0 != *(param->handlePlay))
		{
			alsaStop(*(param->handlePlay));
			*(param->handlePlay) = 0;
			alsaDestroy(*(param->handlePlay));
		}
		alsaInit("0,0",0,2,48000,0);
		alsaStart(*(param->handlePlay));
		alsaSend(*(param->handlePlay),param->outPtr, param->srLength);
	}else{
		alsaSend(*(param->handlePlay),param->outPtr, param->srLength);
		switch(*(param->pauseFlag))
		{
			case PAUSE_RESUME_PALY: 
						alsaStop(*(param->handlePlay));
						break;
			case PAUSE_RESUME_PAUSE:
						while(*(param->pauseFlag) == PAUSE_RESUME_PAUSE);
						if(*(param->pauseFlag) == PAUSE_RESUME_RESUME)
							alsaStart(*(param->handlePlay));
						//	*(param->setPcmFlag) = 1;
						else if((*(param->pauseFlag)==DOWN_VOICE)||\
							(*(param->pauseFlag)==UPPER_VOICE))
							*(param->pauseFlag) = PAUSE_RESUME_PAUSE;
						break;
			case PAUSE_RESUME_RESUME:
			//			*(param->pauseFlag) = 0;
						break;
			case PAUSE_RESUME_STOP:
						alsaStop(*(param->handlePlay));
						*(param->pauseFlag) = 0;
						break;
			case DOWN_VOICE:	
#if 0				
						set_mixer(param->handleMixer,param->handleElement);
						curValue = curVoiceValue(*(param->handleMixer),*(param->handleElement));
						changeVoiceValue(curValue - 5,*(param->handleElement));
						destroyMixter(*(param->handleMixer));
						*(param->pauseFlag) = 0;
//						*(param->setPcmFlag) = 1;
						break;
			case UPPER_VOICE:
						set_mixer(param->handleMixer,param->handleElement);
						curValue = curVoiceValue(*(param->handleMixer),*(param->handleElement));
						changeVoiceValue(curValue + 5,*(param->handleElement));
						destroyMixter(*(param->handleMixer));
						*(param->pauseFlag)  = 0;
//						*(param->setPcmFlag) = 1;
						break;
#endif
			case CHANGE_SONGS:
						alsaStop(*(param->handlePlay));
//						*(param->pauseFlag) = 0;
						break;

			case TTS_CMD_PALY:
						*(param->pauseFlag) = 0;
		}

	}

}

void fflushCommonBuffer(char **cData,int *cLength,int ffFlag)
{
	char cBufLeng[4] = "";
	if(ffFlag == 0)
	{
		while(commonBuff[0]=='Y');
		sprintf(cBufLeng,"%d",*cLength);
		memcpy(commonBuff+1,cBufLeng,4);
		memcpy(commonBuff+5,*cData,*cLength);
		commonBuff[0] = 'Y';
	}else{
		while(commonBuff[0]=='N');
		memcpy(cBufLeng,commonBuff+1,4);
		*cLength = atoi(cBufLeng);
		memcpy(*cData,commonBuff+5,*cLength);
		memset(commonBuff,0,1024*8);
		commonBuff[0] = 'N';
	}
}		
			

void *cmdPlaying(void *arg)
{
	int result;
	struct buffer_input *inptDat = (struct buffer_input *)arg;
	mad_decoder_init(inptDat->decoder,inptDat->buff,input,0,0,output,error,0);
    result = mad_decoder_run(inptDat->decoder, MAD_DECODER_MODE_SYNC);
	*(inptDat->buff->ENDING_FLAG) = 1;
	*(inptDat->buff->SET_PCM_FLAG) = 1;
	printf("#####cmdPLaying	&&&&&&&&#~~~~~~~~~~~~~~~~~~~~~~~~~~~	%lu\n",pthread_self());
}


int decode(struct decode_frame *framInfo,struct buffer *buffer,struct mad_decoder*decoder,int*initFlag)
{
	
#if 0
	struct buffer buffer;
 	struct mad_decoder decoder;
  	int result;

  	buffer.start  = framInfo->frame_data;
  	buffer.length = framInfo->frame_length;
	buffer.handle = NULL;
	buffer.params = NULL;
	buffer.mixer = NULL;
	buffer.element = NULL;
	buffer.SET_PCM_FLAG = framInfo->init_pcm_flag;
	buffer.PAUSE_FLAG = framInfo->control_flag;
	buffer.ENDING_FLAG = framInfo->ending_flag;
	buffer.playMP3 = framInfo->playfunc;
  	mad_decoder_init(&decoder,&buffer,input,0,0,output,error,0);
  	result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
  	mad_decoder_finish(&decoder);
	
 	return result;
	
#endif
	pthread_t playThread;
    pthread_attr_t  attrControl;
	if(*initFlag == 0)
	{	
		buffer->start = (char *)malloc(1024* 4);
		buffer->pre = (unsigned char *)malloc(1024* 4);
		buffer->playing = (unsigned char *)malloc(1024 *8);
		memset(buffer->start,0,4096);
  		memcpy(buffer->start,framInfo->frame_data,framInfo->frame_length);
//		memcpy(buffer->pre,framInfo->frame_data,framInfo->frame_length);
		
  		buffer->length = framInfo->frame_length;
		buffer->lengPre = 0;
		buffer->palyHandle = 0;
//		buffer->mixer = NULL;
//		buffer->element = NULL;
		buffer->SET_PCM_FLAG = framInfo->init_pcm_flag;
		buffer->PAUSE_FLAG = framInfo->control_flag;
		buffer->ENDING_FLAG = framInfo->ending_flag;
		buffer->playMP3 = framInfo->playfunc;
		buffer->gettingBuff = framInfo->pushBuff;
		
		static struct buffer_input inptDat;
		inptDat.buff = buffer;
		inptDat.decoder = decoder;
		
    	pthread_attr_init(&attrControl);
    	pthread_attr_setdetachstate(&attrControl,PTHREAD_CREATE_DETACHED);
		if(pthread_create(&playThread,&attrControl,cmdPlaying,(void*)&inptDat)!=0)
   			PERROR(-1,"\npthread_create error");		
  //		mad_decoder_init(decoder,buffer,input,0,0,output,error,0);
  	//	result = mad_decoder_run(decoder, MAD_DECODER_MODE_SYNC);
		*initFlag = 1;
	}
	else if(*initFlag == -1){
		buffer->length = 0;
		
		mad_decoder_finish(decoder);
		free(buffer->start);
		free(buffer->pre);
		free(buffer->playing);
		
	}else if(*initFlag == 8){
		printf("8888888888888888888888\n");
		buffer->length = framInfo->tts_length;
		buffer->start = framInfo->tts_buffer;
		buffer->lengPre = 8888;
	
//		buffer->mixer = NULL;
//		buffer->element = NULL;
		buffer->SET_PCM_FLAG = framInfo->init_pcm_flag;
		buffer->PAUSE_FLAG = framInfo->control_flag;
		buffer->ENDING_FLAG = framInfo->ending_flag;
		buffer->playMP3 = framInfo->playfunc;

		while(!(*(buffer->ENDING_FLAG)))
		{
			printf("-----------------------------$$$$$$$$$$$$$$$$$$$$8888888888888888888888888888\n");
			sleep(1);
			fflush(stdout);
		}
		mad_decoder_init(decoder,buffer,input,0,0,output,error,0);
		mad_decoder_run(decoder, MAD_DECODER_MODE_SYNC);
		mad_decoder_finish(decoder);
		*(buffer->ENDING_FLAG) = 1;
		*(buffer->SET_PCM_FLAG) = 1;printf("hhhhhhhhhhhhhhhhhhhhhhhhhh\n");
	}
}

