#ifndef _VOICE_INTERFACE_H_
#define _VOICE_INTERFACE_H_
#ifdef __cplusplus
extern "C" {
#endif
typedef void *VoiceHandle;
extern void voiceInit(VoiceHandle *handle);
extern void voiceProcess(VoiceHandle handle,float *input,short *output,float *aec,int *doaDir); 
/*
name : 	increasedB mpo倍数 参考值：6 9 12 （整数） 
	snrxdb snr倍数 9 12 （整数）
	beamScale beam 倍数 1.0 3.0 （浮点数） 
*/
extern void voiceSetParams(VoiceHandle handle,char *name,void *value);
extern void voiceDestroy(VoiceHandle handle);
extern void voiceReset(VoiceHandle handle); 
extern int voiceProcessContainPreparation(VoiceHandle handle,char *input,short *output); 
#ifdef __cplusplus
}
#endif
#endif
