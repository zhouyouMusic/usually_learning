#ifdef USE_NATIVE_ALI

#include "sgn_native_ali_module.h"

#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "native/sgn_native.h"
#include "third/cJSON/cJSON.h"

#include "third/native_corelib/include/audio-align-scorer-export.h"

struct sgn_native_ali {
    struct AudioAlignScorer *gs;
    struct cJSON *param;
    sgn_native_callback_t *cb;
};

struct sgn_native_ali *sgn_native_ali_new(const char *ali_dir)
{
	struct sgn_native_ali * ali = (struct sgn_native_ali *)calloc(1, sizeof(*ali));
    return ali;
}

int sgn_native_ali_delete(struct sgn_native_ali *ali)
{
	if(ali != NULL)free(ali);
    return 0;
}


int sgn_native_ali_start(struct sgn_native_ali *ali, const char *param, sgn_native_callback_t *callback)
{
	int rv = -1;
	char *ref_audio = NULL;//, *new_param = NULL;
	cJSON *jsonItem = NULL, *jsonType = NULL;
	cJSON *jsonParam = cJSON_Parse(param);
	if(!jsonParam)goto end;
	jsonItem = cJSON_GetObjectItem(jsonParam, "refAudio");
	if(!jsonItem)goto end;
	ref_audio = jsonItem->valuestring;

	ali->cb = callback;
	ali->gs = AudioAlignScorerNew(param);
	AudioAlignScorerStartNewReferenceAudio(ali->gs, ref_audio);
	if(jsonParam)cJSON_Delete(jsonParam);
	rv = 0;
end:
    return rv;
}


int sgn_native_ali_feed(struct sgn_native_ali *ali, const void *buf, int buf_size)
{
	int rv = 0;
	AudioAlignScorerAppend(ali->gs, (const short *)buf, buf_size/2);
    return rv;
}

int sgn_native_ali_stop(struct sgn_native_ali *ali)
{
    int rv = 0;
    AudioAlignScorerEnd(ali->gs);
    char const*result = AudioAlignScorerGetOutput(ali->gs);
    if(result != NULL)
    	ali->cb->callback(ali->cb->user_data, SKEGN_MESSAGE_TYPE_JSON, strlen(result), (void *)result);
    else
    	ali->cb->callback(ali->cb->user_data, SKEGN_MESSAGE_TYPE_JSON, strlen("error"), "error");
    AudioAlignScorerDestroy(ali->gs);
    return rv;
}

int sgn_native_ali_cancel(struct sgn_native_ali *ali)
{
    return 0;
}
#endif
