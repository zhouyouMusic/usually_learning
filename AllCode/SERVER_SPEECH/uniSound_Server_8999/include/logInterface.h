#ifndef _LOG_INTERFACE_H_
#define _LOG_INTERFACE_H_
typedef long long int logCtlHandle;

/*****************	FILE_NAME 指定普通日志的文件名	***********************/
/*****************	ERROR_FILE 指定错误日志的文件名	***********************/
/*****************	FILE_PATH 指定日志的路径	***********************/
/*****************	MAX_SIZE 指定每个文件大小，单位字节	***********************/
/*****************	KEEP_NUM 指定路径下最多存放的日志备份数目	***********************/

#define   FILE_NAME   "Server_Runing.log"
#define   ERROR_FILE   "Server_Error.log"
#define   FILE_PATH   "logSystem"
#define   MAX_SIZE	  1024 *10
#define   KEEP_NUM    8


logCtlHandle ErrLogHandle;
logCtlHandle InfoLogHandle;


extern logCtlHandle logInitHandle(char *fileNm,char *filePh,int maxSz,int bkNum);
extern void BASE_ERROR_LOG(logCtlHandle lgHandle,const char *format,...);
extern void BASE_INFO_LOG(logCtlHandle lgHandle,const char *format,...);


#endif
