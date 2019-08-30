#ifndef _ALSA_INTERFACE_H_
#define _ALSA_INTERFACE_H_
#ifdef __cplusplus
extern "C" {
#endif
typedef long long int AlsaHandle;
/*modle = 0 play modle = 1 record*/
extern AlsaHandle alsaInit(char *deviceName,short modle,unsigned int channels,unsigned int rate,unsigned int format);
extern int alsaRead(AlsaHandle handle,char *buffer,int length);
extern void alsaSend(AlsaHandle handle,char *buffer,int length);
extern void alsaDestroy(AlsaHandle handle);
extern void alsaStart(AlsaHandle handle);
extern void alsaStop(AlsaHandle handle);


#ifdef __cplusplus
}
#endif
#endif
