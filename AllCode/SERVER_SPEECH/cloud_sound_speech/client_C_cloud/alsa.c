#ifndef _ANDROID_
#include "alsa.h"
#include "alsaInterface.h"

static int set_hwparams(AlsaContext *context,snd_pcm_access_t access)
{
	unsigned int rrate;
	snd_pcm_uframes_t size;
	int err, dir;
	/* choose all parameters */
	err = snd_pcm_hw_params_any(context->handle, context->hwparams);
	if (err < 0) {
		printf("Broken configuration for playback: no configurations available: %s\n", snd_strerror(err));
		return err;
	}
	/* set hardware resampling */
	err = snd_pcm_hw_params_set_rate_resample(context->handle, context->hwparams, context->resample);
	if (err < 0) {
		printf("Resampling setup failed for playback: %s\n", snd_strerror(err));
		return err;
	}
	/* set the interleaved read/write format */
	err = snd_pcm_hw_params_set_access(context->handle, context->hwparams, access);
	if (err < 0) {
		printf("Access type not available for playback: %s\n", snd_strerror(err));
		return err;
	}
	/* set the sample format */
	err = snd_pcm_hw_params_set_format(context->handle, context->hwparams, context->format);
	if (err < 0) {
		printf("Sample format not available for playback: %s\n", snd_strerror(err));
		return err;
	}
	/* set the count of channels */
	err = snd_pcm_hw_params_set_channels(context->handle, context->hwparams, context->channels);
	if (err < 0) {
		printf("Channels count (%i) not available for playbacks: %s\n", context->channels, snd_strerror(err));
		return err;
	}
	/* set the stream rate */
	rrate = context->rate;
	err = snd_pcm_hw_params_set_rate_near(context->handle, context->hwparams, &rrate, 0);
	if (err < 0) {
		printf("Rate %iHz not available for playback: %s\n", context->rate, snd_strerror(err));
		return err;
	}
	if (rrate != context->rate) {
		printf("Rate doesn't match (requested %iHz, get %iHz)\n", context->rate, err);
		return -EINVAL;
	}
	/* set the buffer time */
	err = snd_pcm_hw_params_set_buffer_time_near(context->handle, context->hwparams, &context->buffer_time, &dir);
	if (err < 0) {
		printf("Unable to set buffer time %i for playback: %s\n", context->buffer_time, snd_strerror(err));
		return err;
	}
	err = snd_pcm_hw_params_get_buffer_size(context->hwparams, &size);
	if (err < 0) {
		printf("Unable to get buffer size for playback: %s\n", snd_strerror(err));
		return err;
	}
	context->buffer_size = size;
	/* set the period time */
	err = snd_pcm_hw_params_set_period_time_near(context->handle, context->hwparams, &context->period_time, &dir);
	if (err < 0) {
		printf("Unable to set period time %i for playback: %s\n", context->period_time, snd_strerror(err));
		return err;
	}
	err = snd_pcm_hw_params_get_period_size(context->hwparams, &size, &dir);
	if (err < 0) {
		printf("Unable to get period size for playback: %s\n", snd_strerror(err));
		return err;
	}
	context->period_size = size;
	/* write the parameters to device */
	err = snd_pcm_hw_params(context->handle, context->hwparams);
	if (err < 0) {
		printf("Unable to set hw params for playback: %s\n", snd_strerror(err));
		return err;
	}
	return 0;
}

static int set_swparams(AlsaContext *context)
{
	int err;
	/* get the current swparams */
	err = snd_pcm_sw_params_current(context->handle, context->swparams);
	if (err < 0) {
		printf("Unable to determine current swparams for playback: %s\n", snd_strerror(err));
		return err;
	}
	/* start the transfer when the buffer is almost full: */
	/* (buffer_size / avail_min) * avail_min */
	err = snd_pcm_sw_params_set_start_threshold(context->handle, context->swparams, (context->buffer_size / context->period_size) * context->period_size);
	if (err < 0) {
		printf("Unable to set start threshold mode for playback: %s\n", snd_strerror(err));
		return err;
	}
	/* allow the transfer when at least period_size samples can be processed */
	/* or disable this mechanism when period event is enabled (aka interrupt like style processing) */
	int period_event = 0;
	err = snd_pcm_sw_params_set_avail_min(context->handle, context->swparams, period_event ? context->buffer_size : context->period_size);
	if (err < 0) {
		printf("Unable to set avail min for playback: %s\n", snd_strerror(err));
		return err;
	}
	/* enable period events when requested */
	if (period_event) {
		err = snd_pcm_sw_params_set_period_event(context->handle, context->swparams, 1);
		if (err < 0) {
			printf("Unable to set period event: %s\n", snd_strerror(err));
			return err;
		}
	}
	/* write the parameters to the playback device */
	err = snd_pcm_sw_params(context->handle, context->swparams);
	if (err < 0) {
		printf("Unable to set sw params for playback: %s\n", snd_strerror(err));
		return err;
	}
	return 0;
}
/*
 *   Underrun and suspend recovery
 */

static int xrun_recovery(snd_pcm_t *handle, int err)
{
	if (err == -EPIPE) {    /* under-run */
		err = snd_pcm_prepare(handle);
		if (err < 0)
			printf("Can't recovery from underrun, prepare failed: %s\n", snd_strerror(err));
		return 0;
	} else if (err == -ESTRPIPE) {
		while ((err = snd_pcm_resume(handle)) == -EAGAIN)
			sleep(1);       /* wait until the suspend flag is released */
		if (err < 0) {
			err = snd_pcm_prepare(handle);
			if (err < 0)
				printf("Can't recovery from suspend, prepare failed: %s\n", snd_strerror(err));
		}
		return 0;
	}
	return err;
}
/*
 *   Transfer method - write only
 */
/*static int write_loop(snd_pcm_t *handle,
		signed short *samples)
{
	double phase = 0;
	signed short *ptr;
	int err, cptr;

		ptr = samples;
		cptr = period_size;
		while (cptr > 0) {
			err = snd_pcm_writei(handle, ptr, cptr);
			if (err == -EAGAIN)
				continue;
			if (err < 0) {
				if (xrun_recovery(handle, err) < 0) {
					printf("Write error: %s\n", snd_strerror(err));
					exit(EXIT_FAILURE);
				}
				break;   //skip one period 
			}
			ptr += err * channels;
			cptr -= err;
		}
	}
	fclose(fp);
}
*/
extern AlsaHandle alsaInit(char *deviceName,short modle,unsigned int channels,unsigned int rate,unsigned int format) {
	AlsaContext *context = (AlsaContext *)malloc(sizeof(AlsaContext));	
	context->rate = rate;
	context->channels = channels;
	context->modle = modle;
	if (modle == 0) {
		//context->buffer_time = 10000;// 500000;
		//context->period_time = 1000;///10000;
		context->buffer_time = 50000;
		context->period_time = 10000;
	} else {
		context->buffer_time = 500000;
		context->period_time = 10000;
	}
	context->resample = 1;
	context->format = SND_PCM_FORMAT_S16_LE;
	snd_pcm_hw_params_alloca(&(context->hwparams));
	int err = -1;
	if (modle == 0) {
		snd_pcm_sw_params_alloca(&(context->swparams));
		if ((err = snd_pcm_open(&(context->handle), deviceName, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
			printf("Playback open error: %s\n", snd_strerror(err));
			return -1;
		}
	} else {
		if ((err = snd_pcm_open(&(context->handle), deviceName, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
			printf("record open error: %s\n", snd_strerror(err));
			return -1;
		}
	}

	if ((err = set_hwparams(context, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		printf("Setting of hwparams failed: %s\n", snd_strerror(err));
		return -1;
	}
	if (modle == 0) {
		if ((err = set_swparams(context)) < 0) {
			printf("Setting of swparams failed: %s\n", snd_strerror(err));
			return -1;
		}
	}
	context->sample = channels * 2;// *context->period_size;
	return (AlsaHandle)(context);	
}
extern int alsaRead(AlsaHandle handle,char *buffer,int length) {
	
	AlsaContext *context = (AlsaContext *)handle;	
		int number = length / context->sample;
		
		int err = snd_pcm_readi(context->handle, buffer, number);
//		int err = snd_pcm_readi(context->handle, buffer, length);
		
		//printf("%d\n",err);
		if (err < 0) {
			if (xrun_recovery(context->handle, err) < 0) {
				printf("Write error: %s\n", snd_strerror(err));
				printf("==============\n");
				return -1;//exit(EXIT_FAILURE);
			}
		}
	return err;
}
extern void alsaStart(AlsaHandle handle) {
	AlsaContext *context = (AlsaContext *)handle;
	int err = snd_pcm_prepare(context->handle);
}
extern void alsaStop(AlsaHandle handle) {
	AlsaContext *context = (AlsaContext *)handle;
	snd_pcm_drain(context->handle);	
}
extern void alsaSend(AlsaHandle handle,char *buffer,int length) {
	AlsaContext *context = (AlsaContext *)handle;
	//int err = snd_pcm_prepare(context->handle);	
		int number = length / context->sample;
		int err = snd_pcm_writei(context->handle, buffer, number);
		//printf("%d\n",err);

		if (err < 0) {
			if (xrun_recovery(context->handle, err) < 0) {
				printf("Write error: %s\n", snd_strerror(err));
				exit(EXIT_FAILURE);
			}
		}
}
extern void alsaDestroy(AlsaHandle handle) {
	AlsaContext *context = (AlsaContext *)handle;	
	snd_pcm_drain(context->handle);	
	snd_pcm_close(context->handle);
/*	snd_pcm_hw_params_free(&context->hwparams);
	if (context->modle == 0) {
		snd_pcm_sw_params_free(&context->swparams);
	}
*/	free(context);
}
#endif
