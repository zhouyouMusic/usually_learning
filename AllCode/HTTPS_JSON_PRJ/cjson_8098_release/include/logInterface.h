#ifndef _LOG_INTERFACE_H_
#define _LOG_INTERFACE_H_
typedef long long int logCtlHandle;

#define   FILE_NAME   "R16_Runing.log"
#define   ERROR_FILE   "R16_Error.log"
#define   FILE_PATH   "logSystem"
#define   MAX_SIZE	  1024 *10
#define   KEEP_NUM    8


logCtlHandle ErrLogHandle;
logCtlHandle InfoLogHandle;


extern logCtlHandle logInitHandle(char *fileNm,char *filePh,int maxSz,int bkNum);
extern void BASE_ERROR_LOG(logCtlHandle lgHandle,const char *format,...);
extern void BASE_INFO_LOG(logCtlHandle lgHandle,const char *format,...);


#endif
