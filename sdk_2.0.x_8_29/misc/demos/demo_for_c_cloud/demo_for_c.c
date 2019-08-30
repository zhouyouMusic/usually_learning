
#include "skegn.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif


static int __stdcall
_callback(const void *usrdata, const char *id, int type, const void *data, int size)
{
    printf("%s - %.*s\n", id, size, (char *)data);
    fflush(stdout);
    return 0;
}

int
main(int argc, char **argv)
{
//     char cfg[] ="{\"appKey\": \"17KouyuTestAppKey\",\"secretKey\": \"17KouyuTestSecretKey\",\"provision\": \"skegn.provision\",\"cloud\": {\"server\": \"ws://api.17kouyu.com:8080\"} }";
	char cfg[] ="{\"appKey\": \"17KouyuTestAppKey\",\"secretKey\": \"17KouyuTestSecretKey\",\"cloud\": {\"sdkCfgAddr\":\"http://update.17kouyu.com/sdk.cfg\"} }";
//	char cfg[] ="{\"appKey\": \"17KouyuTestAppKey\",\"secretKey\": \"17KouyuTestSecretKey\",\"cloud\": {} }";

	char params[] = "{\"coreProvideType\":\"cloud\",\"app\":{\"userId\":\"xxx\"},\"audio\":{\"audioType\":\"wav\",\"sampleRate\":16000,\"channel\":1,\"sampleBytes\":2},\"request\":{\"getParam\":0, \"coreType\":\"sent.eval\",\"refText\":\"I know the place very well\",\"phoneme_output\":0}}";
     int   bytes;
     char  id[64], buf[1024];
     FILE *file;

     struct skegn *engine;

    file = fopen("I.wav", "rb");
    fseek(file, 44, SEEK_SET);	//去除wav音频头

    engine = skegn_new(cfg);
    skegn_start(engine, params, id, (skegn_callback)_callback, NULL);

    while ((bytes = (int)fread(buf, 1, 1024, file))) {
        skegn_feed(engine, buf, bytes);
    }

    skegn_stop(engine);

    //sleep(30); /* wait for result */
    getchar();
    skegn_delete(engine);
    fclose(file);

    return 0;
}
#ifdef __cplusplus
}
#endif
