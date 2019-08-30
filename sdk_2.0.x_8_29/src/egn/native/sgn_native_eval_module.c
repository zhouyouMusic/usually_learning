#ifdef USE_NATIVE_EVAL

#include "sgn_native_eval_module.h"

#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "native/sgn_native.h"
#include "third/cJSON/cJSON.h"

#include "third/native_corelib/include/closed-scorer-export.h"

struct sgn_native_eval {
    struct MinimumEntropyScorer *ds;
    struct cJSON *param;
    sgn_native_callback_t *cb;
};

struct sgn_native_eval *sgn_native_eval_new(const char *res_dir)
{
	struct sgn_native_eval * eval = (struct sgn_native_eval *)calloc(1, sizeof(*eval));
    return eval;
}

int sgn_native_eval_delete(struct sgn_native_eval *eval)
{
	if(eval != NULL)free(eval);
    return 0;
}

int sgn_native_eval_start(struct sgn_native_eval *eval, const char *param, sgn_native_callback_t *callback)
{
	int rv = -1;
	char *ref_text = NULL, *new_param = NULL;
	cJSON *jsonItem = NULL, *jsonType = NULL;
	cJSON *jsonParam = cJSON_Parse(param);
	if(!jsonParam)goto end;
	jsonItem = cJSON_GetObjectItem(jsonParam, "refText");
	if(!jsonItem)goto end;
	ref_text = jsonItem->valuestring;
	jsonType = cJSON_GetObjectItem(jsonParam, "coreType");
	if(jsonType == NULL || strcmp(jsonType->valuestring, "sent.eval")==0)	//不传内核，默认句子
	{
		if((jsonType=cJSON_GetObjectItem(jsonParam, "phoneme_output"))!=NULL && jsonType->type==cJSON_Number){
			cJSON_AddNumberToObject(jsonParam, "phoneme_output", jsonType->valueint);
			if((jsonType=cJSON_GetObjectItem(jsonParam, "dict_type"))==NULL){
				cJSON_AddStringToObject(jsonParam, "dict_type", "KK");
			}
		}else{
			cJSON_AddNumberToObject(jsonParam, "phoneme_output", 0);	// 句子关闭音素纬度输出
		}
//		cJSON_AddStringToObject(jsonParam, "subtype", "sentence");
//		cJSON_AddNumberToObject(jsonParam, "rear_tone", 1);
	}else if(strcmp(jsonType->valuestring, "word.eval") == 0){
		if(cJSON_GetObjectItem(jsonParam, "dict_type") == NULL){	// 单词默认音素字典为KK
			cJSON_AddStringToObject(jsonParam, "dict_type", "KK");
		}
		cJSON_AddStringToObject(jsonParam, "subtype", "word");
		cJSON_AddNumberToObject(jsonParam, "rear_tone", 0);
	}else if(strcmp(jsonType->valuestring, "para.eval") == 0){	// 段落默认关闭音素纬度，开句子得分
		cJSON_AddNumberToObject(jsonParam, "phoneme_output", 0);
		cJSON_AddStringToObject(jsonParam, "subtype", "paragraph");
		if((jsonType=cJSON_GetObjectItem(jsonParam, "paragraph_need_word_score"))!=NULL && jsonType->type==cJSON_Number){	// 默认不返回每个单词得分
			cJSON_AddNumberToObject(jsonParam, "paragraph_need_word_score", jsonType->valueint);
		}
	}else{
		goto end;
	}
	// cJSON_DeleteItemFromObject(jsonParam, "coreType");
	// if(cJSON_GetObjectItem(jsonParam, "slack") == NULL){
	// 	cJSON_AddNumberToObject(jsonParam, "slack", 0.2);
	// }
	new_param = cJSON_PrintUnformatted(jsonParam);
	eval->cb = callback;
	eval->ds = MinimumEntropyScorerNew(new_param);
	MinimumEntropyScorerStartNewPrompt(eval->ds, ref_text);
	if(jsonParam)cJSON_Delete(jsonParam);
	if(new_param)free(new_param);
	rv = 0;
end:
    return rv;
}


int sgn_native_eval_feed(struct sgn_native_eval *eval, const void *buf, int buf_size)
{
	int rv = 0;
	MinimumEntropyScorerAppend(eval->ds, (const short *)buf, buf_size/2);
    return rv;
}

int sgn_native_eval_stop(struct sgn_native_eval *eval)
{
    int rv = 0;
    MinimumEntropyScorerEnd(eval->ds);
    char const*result = MinimumEntropyScorerGetOutput(eval->ds);

    if(result != NULL)
    	eval->cb->callback(eval->cb->user_data, SKEGN_MESSAGE_TYPE_JSON, strlen(result), (void *)result);
    else
    	eval->cb->callback(eval->cb->user_data, SKEGN_MESSAGE_TYPE_JSON, strlen("error"), "error");
    MinimumEntropyScorerDestroy(eval->ds);
    return rv;
}

int sgn_native_eval_cancel(struct sgn_native_eval *eval)
{
    return 0;
}
#endif
