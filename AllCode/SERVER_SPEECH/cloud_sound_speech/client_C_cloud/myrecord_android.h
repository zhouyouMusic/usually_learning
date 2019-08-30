#ifdef _ANDROID_
#ifndef __MYRECORD_ANDROID_H
#define __MYRECORD_ANDROID_H

#include "asoundlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
//#include <unistd.h>


#define FORMAT_PCM 1

#if 0
#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

struct wav_header {
    uint32_t riff_id;
    uint32_t riff_sz;
    uint32_t riff_fmt;
    uint32_t fmt_id;
    uint32_t fmt_sz;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    uint32_t data_id;
    uint32_t data_sz;
};

#endif
typedef struct{
	struct pcm_config config;
	struct pcm *pcm;
	char *buffer;
	unsigned int size;
	unsigned int capturing;
	FILE *file;
}pcm_params;
unsigned int capture_init(pcm_params* params,unsigned int card, unsigned int device,	unsigned int channels, unsigned int rate,enum pcm_format format, unsigned int period_size,unsigned int period_count);
unsigned int capture_process(pcm_params* params);
unsigned int capture_read(pcm_params *params,char *buffer,int size);
unsigned int capture_start(pcm_params *params);
unsigned int capture_stop(pcm_params *params);
void capture_destroy(pcm_params* params);

#endif //__MYRECORD_ANDROID_H
#endif
