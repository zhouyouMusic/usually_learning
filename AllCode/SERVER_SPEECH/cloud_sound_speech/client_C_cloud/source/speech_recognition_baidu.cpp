/***************************************************************************
 * 
 * Copyright (c) 2014 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "speech_recognition.h"
#include "../curl/include/curl/curl.h"
#include "../curl/include/curl/easy.h"

#define MAX_BUFFER_SIZE 1024
#define MAX_BODY_SIZE 1000000


static size_t writefunc(void *ptr, size_t size, size_t nmemb, char **result)
{
    size_t result_len = size * nmemb;
    *result = (char *)realloc(*result, result_len + 1);
    if (*result == NULL)
    {
        printf("realloc failure!\n");
        return 1;
    }
    memcpy(*result, ptr, result_len);
    (*result)[result_len] = '\0';
    return result_len;
} 

void accessID_token(char *apiKey,char *secretKey,char **tokenBuff)
{
	FILE * fpp = NULL;
    char requesCmd[MAX_BUFFER_SIZE];
    char result[MAX_BUFFER_SIZE];
	sprintf(requesCmd,"%s%s%s%s%s%s%s",
					"curl -s ",
						"\"",
					"https://openapi.baidu.com/oauth/2.0/token?grant_type=client_credentials&client_id=",
					apiKey,
					"&client_secret=",
					secretKey,
						"\""	
			);
    fpp = popen(requesCmd, "r");
    fgets(result, MAX_BUFFER_SIZE, fpp);
    pclose(fpp);
	if(NULL != result)
	{
		cJSON * cjsRoot = cJSON_Parse(result);
		cJSON * cjsToken = cJSON_GetObjectItem(cjsRoot,"access_token");
		char * accessId = cJSON_Print(cjsToken);
		strcpy(*tokenBuff,accessId);
		free(accessId);
	}else
		printf("getting accessID failed\n");
}

void speech_recongnize(char *cuid,char *accessId,int rate,int pcmLength,char *pcmBuff,char **speechBuf)
{
	int  i = 0;
	char host[MAX_BUFFER_SIZE] = "";
    char temp[MAX_BUFFER_SIZE] = "";
	sprintf(host,"%s%s%s%s%s",
			"http://vop.baidu.com/server_api",
			"?cuid=",
			cuid,
			"&token=",
			accessId
		);
    CURL *curl;
    CURLcode res;
	char * resultBuf = NULL;
	
    struct curl_slist *headerlist = NULL;
    sprintf(temp,"%s%d","Content-Type: audio/pcm; rate=",rate);
    headerlist = curl_slist_append(headerlist, temp);
	memset(temp,0,MAX_BUFFER_SIZE);
    sprintf(temp,"%s%d","Content-Length: ",pcmLength);
    headerlist = curl_slist_append(headerlist, temp);

    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, host);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30); 
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS,pcmBuff);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, pcmLength);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,&resultBuf);
    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        printf("perform curl error:%d.\n", res);
        return ;
    }
    curl_slist_free_all(headerlist);
    curl_easy_cleanup(curl);
	cJSON * cjsRoot = cJSON_Parse(resultBuf);
	cJSON * subResult = cJSON_GetObjectItem(cjsRoot,"result");
	cJSON * resFilter = cJSON_GetArrayItem(subResult,0);
	char * fResult = cJSON_Print(resFilter);
	char arryBuff[MAX_BUFFER_SIZE] = "";
	int j = 0;
	while(fResult[i]!='\0')
	{	
		if(fResult[i] != '\"')
		{
			arryBuff[j++] = fResult[i];	
		}
		i++;
	}
	arryBuff[j] = '\0';
	strcpy(*speechBuf,arryBuff);
	
	free(resultBuf);
	free(fResult);
	
}


#if 0
int  main (int argc,char* argv[])
{

	char* result = (char*)malloc(MAX_BUFFER_SIZE);
    char *apiKey = "a7cHwiVhGXGI76dPi3R2wnE9";
    char *secretKey = "a1e532cc2cc5c8281233098f04cf8350";
	accessID_token(apiKey,secretKey,&result);
	printf("%s\n",result);	
	free(result);
   
	 if (argc != 2)
    {
        printf("Usage: %s audiofile\n", argv[0]);
        return -1;
    }

    FILE *fp = NULL;
    fp = fopen(argv[1], "r");
    if (NULL == fp)
    {
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    int content_len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *audiodata = (char *)malloc(content_len);
    fread(audiodata, content_len, sizeof(char), fp);


	char* speechBuf = (char*)malloc(MAX_BUFFER_SIZE);
    char *cuid = "18:66:da:2f:dc:1f";
	char *accessId = "23.dd43a82fbeaaeee41a8edf7ce514cef1.2592000.1513231500.514712337-10357154";
	speech_recongnize(cuid,accessId,16000,content_len,audiodata,&speechBuf);
	printf("%s\n",speechBuf);
	
    return 0;
}

#endif
