#ifdef USE_NATIVE_VAD

#include "sgn_native_vad_module.h"

#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "lib/sgn_sound_intensity.h"
#include "native/sgn_native.h"
#include "third/cJSON/cJSON.h"

#include "third/native_corelib/include/vad-export.h"

struct sgn_native_vad {
    struct Vad *ds;
    struct cJSON *param;
    sgn_native_callback_t *cb;
};

struct sgn_native_vad *sgn_native_vad_new(const char *res_dir)
{
	struct sgn_native_vad * vad = (struct sgn_native_vad *)calloc(1, sizeof(*vad));
    return vad;
}

int sgn_native_vad_delete(struct sgn_native_vad *vad)
{
	if(vad != NULL)free(vad);
    return 0;
}

int sgn_native_vad_start(struct sgn_native_vad *vad, const char *param, sgn_native_callback_t *callback)
{
	int rv = -1;
	struct EndpointConfig vadConf = {0.5, 0.6};		//
	cJSON *item = NULL;
	cJSON *obj = cJSON_Parse(param);
	if(obj){
		if((item = cJSON_GetObjectItem(obj, "seek"))!=NULL && item->type==cJSON_Number){
			vadConf.trailing_silence = item->valuedouble/100;
		}
		if((item = cJSON_GetObjectItem(obj, "ref_length"))!=NULL && item->type==cJSON_Number){
			vadConf.minimum_speech_length = item->valuedouble/100;
		}
		cJSON_Delete(obj);
	}
	vad->cb = callback;
	vad->ds = VadNew();
	VadSetEndpointConfig(vad->ds, vadConf);
	VadStart(vad->ds);
	rv = 0;
end:
    return rv;
}

int sgn_native_vad_feed(struct sgn_native_vad *vad, const void *buf, int buf_size)
{
	int status = 0;
	char result[1024] = {0};
	status = VadAppend(vad->ds, (const short *)buf, buf_size/2);
	float vol = sgn_vad_sound_intensity(buf, buf_size);
	sprintf(result, "{\"vad_status\": %d, \"sound_intensity\": %f}", status, vol);
	vad->cb->callback(vad->cb->user_data, 3, strlen(result), (void *)result);
    return status;
}

int sgn_native_vad_stop(struct sgn_native_vad *vad)
{
    int rv = 0;
    VadEnd(vad->ds);
    char const*result = VadGetOutput(vad->ds);

//    if(result != NULL)
//    	vad->cb->callback(vad->cb->user_data, SKEGN_MESSAGE_TYPE_JSON, strlen(result), (void *)result);
//    else
//    	vad->cb->callback(vad->cb->user_data, SKEGN_MESSAGE_TYPE_JSON, strlen("error"), "error");
    VadDestroy(vad->ds);
    return rv;
}

int sgn_native_vad_cancel(struct sgn_native_vad *vad)
{
    return 0;
}
#endif
