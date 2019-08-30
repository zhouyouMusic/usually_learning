#ifdef USE_NATIVE_OPEN

#include "sgn_native_open_module.h"

#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "native/sgn_native.h"
#include "third/cJSON/cJSON.h"

#include "third/native_corelib/include/medium-entropy-scorer-export.h"

struct sgn_native_open {
    struct MediumEntropyScorer *ds;
    struct cJSON *param;
    sgn_native_callback_t *cb;
};

struct sgn_native_open *sgn_native_open_new(const char *res_dir)
{
	struct sgn_native_open * open = (struct sgn_native_open *)calloc(1, sizeof(*open));
    return open;
}

int sgn_native_open_delete(struct sgn_native_open *open)
{
	if(open != NULL)free(open);
    return 0;
}


int sgn_native_open_start(struct sgn_native_open *open, const char *param, sgn_native_callback_t *callback)
{
	int rv = -1;
	char *new_param = NULL;
	cJSON *jsonItem = NULL, *jsonType = NULL;
	cJSON *jsonParam = cJSON_Parse(param);

	jsonType = cJSON_GetObjectItem(jsonParam, "coreType");
	if(jsonType == NULL || strcmp(jsonType->valuestring, "open.eval")==0)	//不传内核，默认句子
	{
		cJSON_AddNumberToObject(jsonParam, "qClass", 1);
		cJSON_AddStringToObject(jsonParam, "type", "open");
		cJSON_AddStringToObject(jsonParam, "setting", "exam");
		//cJSON_AddNumberToObject(jsonParam, "slack", 0.8);
	}else{
		goto end;
	}
	new_param = cJSON_PrintUnformatted(jsonParam);
	open->cb = callback;
	open->ds = MediumEntropyScorerNew(new_param);
	MediumEntropyScorerStart(open->ds);
	if(jsonParam)cJSON_Delete(jsonParam);
	if(new_param)free(new_param);
	rv = 0;
end:
    return rv;
}

int sgn_native_open_feed(struct sgn_native_open *open, const void *buf, int buf_size)
{
	int rv = 0;
	MediumEntropyScorerAppend(open->ds, (const short *)buf, buf_size/2);
    return rv;
}

int sgn_native_open_stop(struct sgn_native_open *open)
{
    int rv = 0;
    MediumEntropyScorerEnd(open->ds);
    char const*result = MediumEntropyScorerGetOutput(open->ds);
    if(*(result+strlen(result)) == '\r'){
    }
    if(*(result+strlen(result)) == '\n'){
	}
    if(result != NULL)
    	open->cb->callback(open->cb->usr_data, SKEGN_MESSAGE_TYPE_JSON, strlen(result), (void *)result);
    else
    	open->cb->callback(open->cb->usr_data, SKEGN_MESSAGE_TYPE_JSON, strlen("error"), "error");
    MediumEntropyScorerDestroy(open->ds);
    return rv;
}

int sgn_native_open_cancel(struct sgn_native_open *open)
{
    return 0;
}
#endif
