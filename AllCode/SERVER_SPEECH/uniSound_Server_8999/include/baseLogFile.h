#ifndef _BASE_LOG_FILE_H_
#define _BASE_LOG_FILE_H_

#include <pthread.h>

typedef struct 
{
	char fileName[50];
	char filePath[20];
	int maxSize;
	int keepNum;
	pthread_mutex_t	 mutexRW;
}LOGDEFINE;

#endif

