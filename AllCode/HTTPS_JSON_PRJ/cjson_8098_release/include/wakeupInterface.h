#ifndef _WAKEUP_INTERFACE_H_
#define _WAKEUP_INTERFACE_H_
#ifdef __cplusplus
extern "C" {
#endif
typedef void *WakeupHandle;
extern void wakeupSingleInit(WakeupHandle *handle,char *mode,char *config); 
extern int wakeupSingleDecode(WakeupHandle handle,char *data,int length); 
extern void wakeupSingleStart(WakeupHandle handle);
extern void wakeupSingleStop(WakeupHandle handle); 
extern void wakeupSingleDestory(WakeupHandle handle); 
#ifdef __cplusplus
}
#endif
#endif
