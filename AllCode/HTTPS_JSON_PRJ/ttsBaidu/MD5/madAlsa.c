# include <stdio.h>
# include <unistd.h>
# include <sys/stat.h>
# include <sys/mman.h>


#include <string.h>
#include<fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <alsa/asoundlib.h>

# include "mad.h"
# include "madAlsa.h"


#include "alsaInterface.h"
/*
 * This is perhaps the simplest example use of the MAD high-level API.
 * Standard input is mapped into memory via mmap(), then the high-level API
 * is invoked with three callbacks: input, output, and error. The output
 * callback converts MAD's high-resolution PCM samples to 16 bits, then
 * writes them to standard output in little-endian, stereo-interleaved
 * format.
 */
//AlsaHandle alsaHandle;
static int decode(unsigned char const *, unsigned long);
int set_pcm(int rateSet,int channelSet);
snd_pcm_t*             handle=NULL;        //PCI设备句柄
snd_pcm_hw_params_t*   params=NULL;//硬件信息和PCM流配置

void  boradMp3(int size, char *mp3Buf,int rate,int channel)
{
  	if(set_pcm(rate,channel)!=0)                 //设置pcm 参数
    {
        printf("set_pcm fialed:\n");
        return;   
   	}
//	alsaHandle = alsaInit("plughw:0,0",0,channel,rate,0);
//	alsaStart(alsaHandle);
//	fp = fopen("./test.pcm","wb");
//	fclose(fp);
//	alsaStop(alsaHandle);
//	alsaDestroy(alsaHandle);
  	decode(mp3Buf, size);
  	snd_pcm_drain(handle);
  	snd_pcm_close(handle);

}


int set_pcm(int rateSet,int channelSet)
{
    int    rc;     
    int  dir=0;
    int rate = rateSet;	//16000  44100            /* 采样频率 44.1KHz*/
    int format = SND_PCM_FORMAT_S16_LE; /*     量化位数 16      */
    int channels = channelSet;     // 1 or 2            /*     声道数 2           */
    int buffer_time = 50000;// 500000;
    int period_time = 10000;///10000;
	 
    rc=snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
        if(rc<0)
        {
                perror("\nopen PCM device failed:");
                exit(1);
        }
    snd_pcm_hw_params_alloca(&params); //分配params结构体
        
    rc=snd_pcm_hw_params_any(handle, params);//初始化params
        if(rc<0)
        {
                perror("\nsnd_pcm_hw_params_any:");
                exit(1);
        }
	
    rc=snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);                                 //初始化访问权限
        if(rc<0)
        {
                perror("\nsed_pcm_hw_set_access:");
                exit(1);

        }
        
    rc=snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);             //设置16位采样精度  
        if(rc<0)
       {
            perror("snd_pcm_hw_params_set_format failed:");
            exit(1);
        } 
        
    rc=snd_pcm_hw_params_set_channels(handle, params, channels);  //设置声道,1表示单声>道，2表示立体声
        if(rc<0)
        {
                perror("\nsnd_pcm_hw_params_set_channels:");
                exit(1);
        }    
        
     rc=snd_pcm_hw_params_set_rate_near(handle, params, &rate, &dir);  //设置>频率
        if(rc<0)
        {
                perror("\nsnd_pcm_hw_params_set_rate_near:");
                exit(1);
        }   
        
	rc=snd_pcm_hw_params_set_buffer_time_near(handle,params,&buffer_time, &dir);
		if(rc<0)
		{
			perror("\nnd_pcm_buffer_errr:	");
			exit(1);
		}
         
	rc=snd_pcm_hw_params_set_period_time_near(handle,params,&period_time, &dir);
		if(rc<0)
		{
			perror("\nnd_pcm_period_errr:	");
			exit(1);
		}
    rc = snd_pcm_hw_params(handle, params);
        if(rc<0)
        {
        perror("\nsnd_pcm_hw_params: ");
        exit(1);
        } 
   
    return 0;              
}

/*
 * This is a private message structure. A generic pointer to this structure
 * is passed to each of the callback functions. Put here any data you need
 * to access from within the callbacks.
 */

struct buffer {
  unsigned char const *start;
  unsigned long length;
};

/*
 * This is the input callback. The purpose of this callback is to (re)fill
 * the stream buffer which is to be decoded. In this example, an entire file
 * has been mapped into memory, so we just call mad_stream_buffer() with the
 * address and length of the mapping. When this callback is called a second
 * time, we are finished decoding.
 */

static
enum mad_flow input(void *data,
            struct mad_stream *stream)
{
  struct buffer *buffer = data;

 printf("this is input\n");
  if (!buffer->length)
    return MAD_FLOW_STOP;

  mad_stream_buffer(stream, buffer->start, buffer->length);

  buffer->length = 0;

  return MAD_FLOW_CONTINUE;
}

/*
 * The following utility routine performs simple rounding, clipping, and
 * scaling of MAD's high-resolution samples down to 16 bits. It does not
 * perform any dithering or noise shaping, which would be recommended to
 * obtain any exceptional audio quality. It is therefore not recommended to
 * use this routine if high-quality output is desired.
 */

static inline
signed int scale(mad_fixed_t sample)
{
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 16));

  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - 16);
}

/*
 * This is the output callback function. It is called after each frame of
 * MPEG audio data has been completely decoded. The purpose of this callback
 * is to output (or play) the decoded PCM audio.
 */

static
enum mad_flow output(void *data,
             struct mad_header const *header,
             struct mad_pcm *pcm)
{
  unsigned int nchannels, nsamples,n;
  mad_fixed_t const *left_ch, *right_ch;

  /* pcm->samplerate contains the sampling frequency */

  nchannels = pcm->channels;
  n=nsamples  = pcm->length;
  left_ch   = pcm->samples[0];
  right_ch  = pcm->samples[1];
  
  unsigned char Output[6912], *OutputPtr;  
  int fmt, wrote, speed, exact_rate, err, dir; 
  
  
//   printf("This is output\n");
   
   
 
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
//	fwrite(OutputPtr,n * 2,1,fp);
//  alsaSend(alsaHandle,OutputPtr,n * 2 * nchannels);
    snd_pcm_writei (handle, OutputPtr, n);  
	  OutputPtr = Output;     

  return MAD_FLOW_CONTINUE;
}

/*
 * This is the error callback function. It is called whenever a decoding
 * error occurs. The error is indicated by stream->error; the list of
 * possible MAD_ERROR_* errors can be found in the mad.h (or stream.h)
 * header file.
 */

static
enum mad_flow error(void *data,
            struct mad_stream *stream,
            struct mad_frame *frame)
{
  struct buffer *buffer = data;
  printf("this is mad_flow error\n");
  fprintf(stderr, "decoding error 0x%04x (%s) at byte offset %ld\n",
      stream->error, mad_stream_errorstr(stream),
      stream->this_frame - buffer->start);

  /* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */

  return MAD_FLOW_CONTINUE;
}

/*
 * This is the function called by main() above to perform all the decoding.
 * It instantiates a decoder object and configures it with the input,
 * output, and error callback functions above. A single call to
 * mad_decoder_run() continues until a callback function returns
 * MAD_FLOW_STOP (to stop decoding) or MAD_FLOW_BREAK (to stop decoding and
 * signal an error).
 */

static
int decode(unsigned char const *start, unsigned long length)
{
  struct buffer buffer;
  struct mad_decoder decoder;
  int result;

  /* initialize our private message structure */

  buffer.start  = start;
  buffer.length = length;

  /* configure input, output, and error functions */

  mad_decoder_init(&decoder, &buffer,
           input, 0 /* header */, 0 /* filter */, output,
           error, 0 /* message */);

  /* start decoding */

  result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

  /* release the decoder */

  mad_decoder_finish(&decoder);

  return result;
}
 

