#include "mad.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "alsaInterface.h"
FILE *fp;
char *databuffer;
//AlsaHandle play;
FILE *outfp;
typedef struct _BUFFER_{
	char const *start;
	long length;
}Buffer;
static enum mad_flow input(void *data,struct mad_stream *stream) {
	int unproc_data_size = stream->bufend - stream->next_frame;
	//	printf("un size = %d\n",unproc_data_size);
	Buffer *buffer = data;
	memcpy(buffer->start,buffer->start + 1024 - unproc_data_size,unproc_data_size);
	int size = fread(databuffer + unproc_data_size,1,1024 - unproc_data_size,fp);
	printf("size = %d\n",size);
	if (size <= 0) {
		return MAD_FLOW_STOP;
	}
	buffer->length = unproc_data_size + size;
	mad_stream_buffer(stream,buffer->start,buffer->length);
	printf("%d\n",buffer->length);
	return MAD_FLOW_CONTINUE;
}
static int scale(mad_fixed_t sample) {
	sample += (1L << (MAD_F_FRACBITS - 16));
	if (sample >= MAD_F_ONE) {
		sample = MAD_F_ONE - 1;
	} else if (sample < -MAD_F_ONE) {
		sample = -MAD_F_ONE;
	}
	return sample >> (MAD_F_FRACBITS - 15);
}
static enum mad_flow output(void *data,struct mad_header const *header,struct mad_pcm *pcm) {
	/*	int nsamples = pcm->length;
		mad_fixed_t const *left_ch = pcm->samples[0];
		short buf[nsamples];
		int i = 0;
		while (nsamples--) {
		int sample = scale(*left_ch++);
		buf[i++] = sample & 0xFFFF;
		}
		fwrite(pcm->samples[0],pcm->length * 4,1,outfp);
	//alsaSend(play,(char *)buf,i * 2);
	//	Buffer *buffer = data;
	//	buffer->onResult((char *)buf,i * 2,buffer->phandle,buffer->handle);

	 */
	unsigned int nchannels, nsamples;
	mad_fixed_t const *left_ch, *right_ch;
	// pcm->samplerate contains the sampling frequency
	nchannels = pcm->channels;
	nsamples = pcm->length;
	left_ch = pcm->samples[0];
	right_ch = pcm->samples[1];
	short buf[nsamples *2];
	int i = 0;
	//printf(">>%d\n", nsamples);
	while (nsamples--) {
		signed int sample;
		// output sample(s) in 16-bit signed little-endian PCM
		sample = scale(*left_ch++);
		buf[i++] = sample & 0xFFFF;
		if (nchannels == 2) {
			sample = scale(*right_ch++);
			buf[i++] = sample & 0xFFFF;
		}
	}
	//fprintf(stderr, ".");
	fwrite(&buf[0], i * 2,1,outfp);
	//alsaSend(play,(char *)buf,i * 2);
	return MAD_FLOW_CONTINUE;


}
int main() {
	outfp = fopen("./test.pcm","wb");
//	play = alsaInit("plughw:0,0",0,2,44100,0);
	databuffer = (char *)malloc(1024);
	fp = fopen("test.mp3","rb");
	struct mad_decoder decoder;
	Buffer buffer;
	buffer.start =databuffer;
	buffer.length = fread(databuffer,1,1024,fp);
	mad_decoder_init(&decoder,&buffer,input,NULL,NULL,output,NULL,NULL);
	mad_decoder_run(&decoder,MAD_DECODER_MODE_SYNC);
	mad_decoder_finish(&decoder);
}
