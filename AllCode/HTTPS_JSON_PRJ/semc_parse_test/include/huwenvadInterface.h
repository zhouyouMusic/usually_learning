#ifndef _HUWEN_VAD_INTERFACE_H_
#define _HUWEN_VAD_INTERFACE_H_
#ifdef __cplusplus
extern "C" {
#endif
typedef struct _HUWEN_VAD_HANDLE_{
	void *handle;
}HuwenVADHandle;
extern void huwenVadInit(HuwenVADHandle *handle,int fs,float startscores,float endscores,int startFrameNumber,int endFrameNumber); 
extern int huwenVadFeedData(HuwenVADHandle *handle,char *data,int length,float *scores); 
extern int huwenVadFeedFData(HuwenVADHandle *handle,float *data,int length,float *scores);
extern void *huwenVadGet(HuwenVADHandle *handle);
extern void huwenVadreset(HuwenVADHandle *handle);
extern void huwenVadDestroy(HuwenVADHandle *handle);
#ifdef __cplusplus
}
#endif
#endif 
