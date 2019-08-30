#ifndef _ALSA_RECORD_H_
#define _ALSA_RECORD_H_
#ifdef __cplusplus
extern "C"{
#endif
#include <alsa/asoundlib.h>
typedef struct _ALSA_CONTEXT_{
	int modle;
	unsigned int rate;
	unsigned int channels;
	snd_pcm_format_t format;
	unsigned int buffer_time;
	unsigned int period_time;
	int resample;
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *hwparams;
	snd_pcm_sw_params_t *swparams;
	snd_pcm_sframes_t buffer_size;
	snd_pcm_sframes_t period_size;
	int sample;
}AlsaContext;



#ifdef __cplusplus
}
#endif
#endif
