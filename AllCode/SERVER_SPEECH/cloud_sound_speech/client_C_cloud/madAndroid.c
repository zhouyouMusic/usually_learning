#include <stdio.h>
#include "stdlib.h"
#include "mad.h"

FILE *fm;
FILE *fp;

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

static enum mad_flow input(void *data,struct mad_stream *stream)
{
  struct buffer *buffer = data;

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

static inline signed int scale(mad_fixed_t sample)
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

static enum mad_flow output(void *data,struct mad_header const *header,struct mad_pcm *pcm)
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

	fwrite(OutputPtr,n,4,fp);
	OutputPtr = Output;     

  return MAD_FLOW_CONTINUE;
}

/*
 * This is the error callback function. It is called whenever a decoding
 * error occurs. The error is indicated by stream->error; the list of
 * possible MAD_ERROR_* errors can be found in the mad.h (or stream.h)
 * header file.
 */

static enum mad_flow error(void *data,struct mad_stream *stream,struct mad_frame *frame)
{

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

static int decode(unsigned char const *start, unsigned long length)
{
  struct buffer buffer;
  struct mad_decoder decoder;
  int result;

	int ret  = 0;
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
 

int main()
{
	fp = fopen("mad.pcm","wb");
	fm = fopen("mm.mp3","rb");
	fseek(fm,0,SEEK_END);
	
	long length = ftell(fm);
	fseek(fm,0,SEEK_SET);
	char *buff = (char*)malloc(length);
	fread(buff,length,1,fm);
	decode(buff,length);
	
}
