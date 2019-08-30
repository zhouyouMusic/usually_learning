#ifdef _ANDROID_
#include "alsaInterface.h"
#include "myrecord_android.h"


extern AlsaHandle alsaInit(char *devName,short modle,unsigned int channels,unsigned int rate,unsigned int format) {
	pcm_params *params = (pcm_params *)malloc(sizeof(pcm_params));
	char deviceName[strlen(devName)];
	memcpy(deviceName,devName,strlen(devName) + 1);
	char *tmp = strtok(deviceName,",");
	int card = atoi(tmp);
	tmp = strtok(NULL,",");
	int device = atoi(tmp);
	printf("%d %d\n",card,device);
//unsigned int capture_init(pcm_params* params,unsigned int card, unsigned int device,	unsigned int channels, unsigned int rate,enum pcm_format format, unsigned int period_size,unsigned int period_count)
//{	
	#ifdef READFILE
	FILE* file = fopen("./recordma.pcm","wb");
	if (!file) {
		fprintf(stderr, "Unable to create file '%s'\n", "./record.wav");
		return 1;
	}
	#endif

	memset(&(params->config), 0, sizeof(params->config));
	params->config.channels = channels;
	params->config.rate = rate;
	params->config.period_size = 512;//period_size;
	params->config.period_count = 48;//period_count;
	params->config.format = PCM_FORMAT_S16_LE;//format;
	params->config.start_threshold = 0;
	params->config.stop_threshold = 0;
	params->config.silence_threshold = 0;

	params->capturing = 1;
	#ifdef READFILE
	params->file = file;
	#endif
//	printf("period_size=%d period_count=%d\n",period_size,period_count);
	if (modle == 1) {
		params->pcm = pcm_open(card, device, PCM_IN, &(params->config));
	} else {
		params->pcm = pcm_open(0, 0, PCM_OUT, &(params->config));
	}
	if (!params->pcm) {
		printf("params-pcm == NULL\n");
	}
	if (!params->pcm || !pcm_is_ready(params->pcm)) {
		fprintf(stderr, "Unable to open PCM device ....(%s)\n",
				pcm_get_error(params->pcm));
		return 0;
	}

	params->size = pcm_frames_to_bytes(params->pcm, pcm_get_buffer_size(params->pcm)) ;
	params->buffer = malloc(params->size);
	if (!params->buffer) {
		fprintf(stderr, "Unable to allocate %d bytes\n",params->size);
		free(params->buffer);
		pcm_close(params->pcm);
		return 0;
	}

//	printf("Capturing sample: %u ch, %u hz, %u bit\n", channels, rate,pcm_format_to_bits(format));
	return params;
}
extern void alsaStart(AlsaHandle handle) {
	pcm_params *params = (pcm_params *)handle;
	pcm_start(params->pcm);
//unsigned int capture_start(pcm_params *params) {
//	return pcm_start(params->pcm);
//	return pcm_prepare(params->pcm);
}
extern void alsaStop(AlsaHandle handle) {
	pcm_params *params = (pcm_params *)handle;
	pcm_stop(params->pcm);
//unsigned int capture_stop(pcm_params *params) {
//	return pcm_stop(params->pcm);
//	return pcm_prepare(params->pcm);
}
extern void alsaSend(AlsaHandle handle,char *buffer,int length) {
	pcm_params *params = (pcm_params *)handle;
	return pcm_write(params->pcm, buffer,length);
} 
extern int alsaRead(AlsaHandle handle,char *buffer,int length) {
	pcm_params *params = (pcm_params *)handle;
//unsigned int capture_read(pcm_params *params,char *buffer,int size) {
	return pcm_read(params->pcm, buffer,length);
}
unsigned int capture_process(pcm_params* params){


#if 0 
	if(params->capturing && !pcm_read_ex(params->pcm,params->buffer,params->size)){
		unsigned int bytes_read = 0;
#ifdef READFILE
		if (fwrite(params->buffer, 1, params->size, params->file) != params->size) {
			fprintf(stderr,"Error capturing sample\n");
			break;
		}
#endif
		bytes_read += params->size;
	}
#endif

#if 1
	unsigned int bytes_read = 0;
	int i=200;
	while(params->capturing && !pcm_read(params->pcm,params-> buffer,params-> size) && i--) {
		unsigned int bytes_read = 0;
#ifdef READFILE
		if (fwrite(params->buffer, 1, params->size, params->file) != params->size) {
			fprintf(stderr,"Error capturing sample\n");
			break;
		}
#endif
		bytes_read += params->size;
	}

#endif

	return pcm_bytes_to_frames(params->pcm, bytes_read);
}
extern void alsaDestroy(AlsaHandle handle) {
	pcm_params *params = (pcm_params *)handle;
//void capture_destroy(pcm_params* params){
	free(params->buffer);
	pcm_close(params->pcm);
	free(params);
}
#endif
