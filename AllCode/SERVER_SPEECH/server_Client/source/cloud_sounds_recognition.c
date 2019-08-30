#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/time.h>

#include "libusc.h"
#include "cloud_sounds_recognition.h"

void init_speech_recognition(USC_HANDLE* handle,struct recog_params * recogParam)
{
	pthread_mutex_lock(&MUTEX_CLOUD_SPEECH);
 	int ret = usc_create_service(handle);
	if (ret != USC_ASR_OK) {
		fprintf(stderr, "usc_create_service_ext error %d\n", ret);
		pthread_mutex_unlock(&MUTEX_CLOUD_SPEECH);
		return;
	}
    ret = usc_set_option((*handle), USC_OPT_ASR_APP_KEY, recogParam->appKey);
    ret = usc_set_option((*handle), USC_OPT_USER_SECRET, recogParam->secretKey);
	ret = usc_login_service(*handle);
    ret = usc_set_option((*handle), USC_OPT_INPUT_AUDIO_FORMAT, recogParam->audioFormat);
    ret = usc_set_option((*handle), USC_OPT_RECOGNITION_FIELD, recogParam->domain);	
    if (recogParam->IsUseNlu)
          ret = usc_set_option((*handle), USC_OPT_NLU_PARAMETER, recogParam->NluBuff);
	pthread_mutex_unlock(&MUTEX_CLOUD_SPEECH);
}

int start_speech_recognition(USC_HANDLE handle)
{
	pthread_mutex_lock(&MUTEX_CLOUD_SPEECH);
		int ret = usc_start_recognizer(handle);
	pthread_mutex_unlock(&MUTEX_CLOUD_SPEECH);
	BASE_INFO_LOG(InfoLogHandle,"%s","start reg~~~\n");
	return ret;
}
int stop_speech_recognition(USC_HANDLE handle,int sockfd)
{
	int y,m,d,h,n,s;
	time_t now;
	struct tm * ptm;
	now = time(NULL);
	ptm = localtime(&now);
	y = ptm->tm_year+1900;
	m = ptm->tm_mon+1;
	d = ptm->tm_mday;
	h = ptm->tm_hour;
	n = ptm->tm_min;
	s = ptm->tm_sec;
	pthread_mutex_lock(&MUTEX_CLOUD_SPEECH);
		int ret = usc_stop_recognizer(handle);
	pthread_mutex_unlock(&MUTEX_CLOUD_SPEECH);
	if(ret != 0)		return ret;
	struct post_content sendData = {0};	
	sendData.contType = 2;
	pthread_mutex_lock(&MUTEX_CLOUD_SPEECH);
		strcpy(sendData.buff,usc_get_result(handle));
	pthread_mutex_unlock(&MUTEX_CLOUD_SPEECH);
	BASE_INFO_LOG(InfoLogHandle,"%s%s%s","stop reg~&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&",sendData.buff,"\n");
	BASE_INFO_LOG(InfoLogHandle,"%d%s%d%s%d%s%d%s%d%s%d%s",y,"-",m,"-",d,"-",h,"-",n,"-",s,"\n");
	send(sockfd,&sendData,sizeof(struct post_content),0);
	return ret;
}
void destroy_speech_recognition(USC_HANDLE handle)
{	
	pthread_mutex_lock(&MUTEX_CLOUD_SPEECH);
		usc_release_service(handle);
		handle = 0;
	pthread_mutex_unlock(&MUTEX_CLOUD_SPEECH);
	
}

void automatic_speech_recognition(USC_HANDLE handle,char *pcmBuff,int pcmLength,int sockfd)
{

	struct post_content sendData = {0};
	pthread_mutex_lock(&MUTEX_CLOUD_SPEECH);
		int ret = usc_feed_buffer(handle,pcmBuff,pcmLength);
	pthread_mutex_unlock(&MUTEX_CLOUD_SPEECH);
	if (ret == USC_RECOGNIZER_PARTIAL_RESULT ||
		ret == USC_RECOGNIZER_SPEAK_END) {	
		pthread_mutex_lock(&MUTEX_CLOUD_SPEECH);
			strcpy(sendData.buff,usc_get_result(handle));
		pthread_mutex_unlock(&MUTEX_CLOUD_SPEECH);
		sendData.contType = 1;
		
		BASE_INFO_LOG(InfoLogHandle,"%s%s",sendData.buff,"    feed result&&&&&&&&&&&&&&&\n");
		send(sockfd,&sendData,sizeof(struct post_content),0);
	}
	else if (ret < 0) {		
		fprintf(stderr, "usc_feed_buffer error %d\n", ret);
		BASE_ERROR_LOG(ErrLogHandle,"%s","usc_feed_buffer error\n");
		return;
	}
}

