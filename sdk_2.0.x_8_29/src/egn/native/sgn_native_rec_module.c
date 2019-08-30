#ifdef USE_NATIVE_REC

#include "sgn_native_rec_module.h"

#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "native/sgn_native.h"
#include "third/cJSON/cJSON.h"

#include "third/native_corelib/include/grammar-scorer-export.h"

#define sgn_native_rec_REFTEXT_EXTLEN 4 * 1024

struct sgn_native_rec {
    struct GrammarScorer *gs;
    struct cJSON *param;
    sgn_native_callback_t *cb;
};


struct sgn_native_rec *sgn_native_rec_new(const char *res_dir)
{
	struct sgn_native_rec * rec = (struct sgn_native_rec *)calloc(1, sizeof(*rec));
    return rec;
}

int sgn_native_rec_delete(struct sgn_native_rec *rec)
{
	if(rec != NULL)free(rec);
    return 0;
}

int sgn_native_rec_start(struct sgn_native_rec *rec, const char *param, sgn_native_callback_t *callback)
{
	int rv = -1;
	char *ref_text = NULL;//, *new_param = NULL;
	cJSON *jsonItem = NULL, *jsonType = NULL;
	cJSON *jsonParam = cJSON_Parse(param);
	if(!jsonParam)goto end;
	jsonItem = cJSON_GetObjectItem(jsonParam, "refText");
	if(!jsonItem)goto end;
	ref_text = jsonItem->valuestring;

	rec->cb = callback;
	rec->gs = GrammarScorerNew();
	GrammarScorerStartSingleChoice(rec->gs, ref_text);
	if(jsonParam)cJSON_Delete(jsonParam);
	rv = 0;
end:
    return rv;
}


int sgn_native_rec_feed(struct sgn_native_rec *rec, const void *buf, int buf_size)
{
	int rv = 0;
	GrammarScorerAppend(rec->gs, (const short *)buf, buf_size/2);
    return rv;
}

int sgn_native_rec_stop(struct sgn_native_rec *rec)
{
    int rv = 0;
    GrammarScorerEnd(rec->gs);
    char const*result = GrammarScorerGetOutput(rec->gs);
    if(result != NULL)
    	rec->cb->callback(rec->cb->user_data, SKEGN_MESSAGE_TYPE_JSON, strlen(result), (void *)result);
    else
    	rec->cb->callback(rec->cb->user_data, SKEGN_MESSAGE_TYPE_JSON, strlen("error"), "error");
    GrammarScorerDestroy(rec->gs);
    return rv;
}

int sgn_native_rec_cancel(struct sgn_native_rec *rec)
{
    return 0;
}
#endif
