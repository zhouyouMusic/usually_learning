#ifndef _JS_WAKEUP_H_
#define _JS_WAKEUP_H_
#ifdef __cplusplus
extern "C" {
#endif
/**
 * 定义错误类型
 */
typedef struct JSErrorInfo {
int num; //错误编号
int type; //错误类型
char* errorMessage; //错误信息
} ErrorInfo;

typedef void (*tInit)(int);
typedef void (*tRecordReleased)();
typedef void (*tReadyForSpeech)();
typedef void (*tWakeupError)(ErrorInfo*);
typedef void (*tBeginningOfSpeech)();
typedef void (*tEndOfSpeech)(long);
typedef void (*tWakeup)(char*);
typedef void (*tRmsChanged)(float);
typedef void (*tBufferReceived)(char*,long);

typedef struct JSCWakeupInterface {
tInit lInit;
tRecordReleased lRecordReleased;
tReadyForSpeech lReadyForSpeech;
tWakeupError lWakeupError;
tBeginningOfSpeech lBeginningOfSpeech;
tEndOfSpeech lEndOfSpeech;
tWakeup lWakeup;
tRmsChanged lRmsChanged;
tBufferReceived lBufferReceived;
} WakeupInterface;

extern void wakeupInit(long*context,char *config,char *model,char *model1);
extern void wakeupSetLister(long context,WakeupInterface *interface);
extern void wakeupDestory(long context);
extern void wakeupStart(long context);
extern void wakeupStop(long context);
extern void wakeupfeedData(long context,char *data,int length);
#ifdef __cplusplus
}
#endif
#endif
