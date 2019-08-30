#ifndef _VOICE_INTERFACE_H_
#define _VOICE_INTERFACE_H_
#ifdef __cplusplus
extern "C" {
#endif
typedef void *VoiceHandle;
extern void voiceInit(VoiceHandle *handle);
extern void voiceProcess(VoiceHandle handle,float *input,short *output,float *aec,int *doaDir); 
extern void voiceDestroy(VoiceHandle handle);
extern void voiceReset(VoiceHandle handle); 
#ifdef __cplusplus
}
#endif
#endif
