#include "skegn.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif


int flagCall = 0;
static int count = 0;
static int __stdcall
_Cdcallback(const void *usrdata, const char *id, int type, const void *data, int size)
{

	printf("Cd callBack-----------------------------------------------\n");
	printf("%s - %.*s\n", id, size, (char *)data);
	flagCall = 1;	
	fflush(stdout);
    return 0;
    
}


static int __stdcall
_Nvcallback(const void *usrdata, const char *id, int type, const void *data, int size)
{
	printf("Nv callBack-----------------------------------------------\n");
    printf("%s - %.*s\n", id, size, (char *)data);
	fflush(stdout);
//	flagCall = 2;
    return 0;
}



int main(int argc, char **argv)
{


	 char cfg[] ="{\"appKey\": \"149500538200000d\",\
					 \"secretKey\": \"99c24cd3b887c45e2560173d639fbeac\",\
					 \"provision\": \"skegn.provision\",\
					 \"cloud\": {\
					 \"server\": \"ws://api.17kouyu.com:8080\"\
					 },\
					 \"native\":\"small_res_1.2.9_2.enc\"\
				 }";



  
     char params[] = "{\"coreProvideType\":\"cloud\",\"app\":{\"userId\":\"xxx\"},\"audio\":{\"audioType\":\"wav\",\"sampleRate\":16000,\"channel\":1,\"sampleBytes\":2},\"request\":{\"getParam\":0, \"coreType\":\"sent.eval\",\"refText\":\"I know the place very well\",\"phoneme_output\":0}}";


	char paramsNative[] = "{\"coreProvideType\":\"native\",\"app\":{\"userId\":\"xxx\"},\"audio\":{\"audioType\":\"wav\",\"sampleRate\":16000,\"channel\":1,\"sampleBytes\":2},\"request\":{\"getParam\":0, \"coreType\":\"sent.eval\",\"refText\":\"I know the place very well\",\"phoneme_output\":0}}";


	

	 int   bytes;

     char  id[64], buf[1024];
     FILE *file;
	 file = fopen("I.wav", "rb");



struct skegn *engineNv; 
engineNv = skegn_new(cfg);



int i = 0;



for(i = 0; i < 1;i++)
{
#if 1
	fseek(file, 44, SEEK_SET);	//

	skegn_start(engineNv, paramsNative, id, (skegn_callback)_Nvcallback, NULL);

	while ((bytes = (int)fread(buf, 1, 1024, file))) {
		skegn_feed(engineNv, buf, bytes);
	}
	

	skegn_stop(engineNv);

#endif

#if 0
//sleep(2);
fseek(file, 44, SEEK_SET);

flagCall = 0;	
skegn_start(engineNv, params, id, (skegn_callback)_Cdcallback, NULL);

    while ((bytes = (int)fread(buf, 1, 1024, file))) {
        skegn_feed(engineNv, buf, bytes);
    }

 skegn_stop(engineNv);

while(flagCall==0);

#endif


}

while(1);
skegn_delete(engineNv);


    return 0;
  
}
#ifdef __cplusplus
}
#endif
