#ifndef _CMD_REQUEST_H_
#define _CMD_REQUEST_H_ 
typedef long long int SemcParseHandle;
typedef long long int HwSoundHandle;

extern SemcParseHandle semcParseHandInit();

extern void semcParseResponse(char *quesCmd,SemcParseHandle semcPrHand,char **smdRlt);


extern HwSoundHandle hwSoundHandleInit(char *ipAr,int port,void (*func_res)(char*));

extern void hwSoundRecgStart(HwSoundHandle usdHandle);

extern void hwSoundRecgStop(HwSoundHandle usdHandle);

extern void hwSoundFeedData(HwSoundHandle usdHandle,char *feedBuff,int feedLen);

extern void hwSoundCancel(HwSoundHandle usdHandle);

extern void hwSoundDestroy(HwSoundHandle usdHandle);


#endif

