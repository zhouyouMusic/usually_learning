#ifdef USE_NATIVE

#include "sgn_event.h"
#include "sgn_native.h"
//#include "sgn/data/sgn_save.h"
#include "native/sgn_auth.h"
#include "third/cJSON/cJSON.h"
#include <string.h>
#include <stdlib.h>
#include <zlib.h>
#include "third/xxtea/sgn_secure_code.h"
#include "third/native_corelib/include/common-utils.h"
#ifdef USE_NATIVE_EVAL
#include "native/sgn_native_eval_module.h"
#endif
#ifdef USE_NATIVE_REC
#include "native/sgn_native_rec_module.h"
#endif
#ifdef USE_NATIVE_ALI
#include "native/sgn_native_ali_module.h"
#endif
#ifdef USE_NATIVE_VAD
#include "native/sgn_native_vad_module.h"
#endif
#ifdef USE_NATIVE_OPEN
#include "native/sgn_native_open_module.h"
#endif
enum {
	SKEGN_NATIVE_EVAL  = 0,
#ifdef USE_NATIVE_REC
	SKEGN_NATIVE_REC,
#endif
#ifdef USE_NATIVE_ALI
	SKEGN_NATIVE_ALI,
#endif
#ifdef USE_NATIVE_VAD
    SKEGN_NATIVE_VAD,
#endif
#ifdef USE_NATIVE_OPEN
	SKEGN_NATIVE_OPEN,
#endif
    SKEGN_NATIVE_MAX
};

typedef void * (*sgn_native_module_new_func)(const char *cfg);
typedef int (*sgn_native_module_delete_func)(void *m);
typedef int (*sgn_native_module_start_func)(void *m, const char *param, sgn_native_callback_t *callback);
typedef int (*sgn_native_module_feed_func)(void *m, const void *data, int size);
typedef int (*sgn_native_module_stop_func)(void *m);
typedef int (*sgn_native_module_cancel_func)(void *m);

static void * sgn_pseudo_new(const char *cfg) {return NULL;}
static int    sgn_pseudo_delete(void *m) {return 0;}
static int    sgn_pseudo_start(void *m, const char *param, sgn_native_callback_t *callback) {return 0;}
static int    sgn_pseudo_feed(void *m, const void *data, int size) {return 0;}
static int    sgn_pseudo_stop(void *m) {return 0;}
static int    sgn_pseudo_cancel(void *m) {return 0;}


static int res_init_flag = 0;


#define SGN_NATIVE_MODULE(name, protected, module) \
{\
    name,\
    protected,\
    (sgn_native_module_new_func)    module##_new,\
    (sgn_native_module_delete_func) module##_delete,\
    (sgn_native_module_start_func)  module##_start,\
    (sgn_native_module_feed_func)   module##_feed,\
    (sgn_native_module_stop_func)   module##_stop,\
    (sgn_native_module_cancel_func) module##_cancel\
}
struct sgn_native_module {
    char                           *name;   /* coreType or coreType prefix */
    unsigned int                    protected:1; /* need auth or not */
    sgn_native_module_new_func      new;
    sgn_native_module_delete_func   delete;
    sgn_native_module_start_func    start;
    sgn_native_module_feed_func     feed;
    sgn_native_module_stop_func     stop;
    sgn_native_module_cancel_func   cancel;
} SGN_NATIVE_MODULES[] = {
#ifdef USE_NATIVE_EVAL
    SGN_NATIVE_MODULE("word.eval", 1, sgn_native_eval),    //sgn_eval_new
//    SGN_NATIVE_MODULE("sent.eval", 1, sgn_native_eval),
//    SGN_NATIVE_MODULE("para.eval", 1, sgn_native_eval),
#endif
#ifdef USE_NATIVE_REC
    SGN_NATIVE_MODULE("choice.rec", 1, sgn_native_rec),
#endif
#ifdef USE_NATIVE_ALI
    SGN_NATIVE_MODULE("align.eval", 1, sgn_native_ali),
#endif
#ifdef USE_NATIVE_OPEN
    SGN_NATIVE_MODULE("open.eval", 1, sgn_native_open),
#endif
    SGN_NATIVE_MODULE("pseudo", 0, sgn_pseudo)
};

struct sgn_native {
    sgn_native_callback_t callback;
    event_t *event;
    int       result_type;
    char      *cur_tokenid;
    int       i;
//    sgn_save_s *sv;
#ifdef USE_NATIVE_VAD
    struct sgn_native_vad *vad;
#endif
    void      *instances[];
};

static struct res_data *decode_resfile(const char *file_path)
{
	char filenumber = 0;
	char tmp_filename[24] = {0};
	unsigned long aft_datalen = 0, dec_data_len = 0, bef_cmp_len = 0;;
	unsigned char *bef_cmp = NULL;  unsigned char *aft_cmp = NULL;
	unsigned char *bef_enc = NULL;  unsigned char *aft_enc = NULL;
	char cmp = 0;
	FILE *res = fopen(file_path, "rb");
	if(res == NULL)return NULL;
	fread(&filenumber, 1, 1, res);
	struct res_data *res_data_head = NULL;
	struct res_data **p = &res_data_head;
	while(filenumber)
	{
		fread(tmp_filename, 1, 24, res);
		fread(&cmp, 1, 1, res);
		fread(&aft_datalen, 1, 4, res);
		aft_enc = (unsigned char *)malloc(aft_datalen);
		fread(aft_enc, 1, aft_datalen, res);
		dec_data_len = sgn_secure_code(aft_enc, aft_datalen, " Fuck u crak", 9, &bef_enc, 'd');	// 解密
		bef_cmp_len = dec_data_len*cmp;
		bef_cmp = (unsigned char *)malloc(bef_cmp_len);
		uncompress(bef_cmp, &bef_cmp_len, bef_enc, dec_data_len);								// 解压缩

		//将解密好的数据放到链表上
		*p = (struct res_data *)malloc(sizeof(**p)+bef_cmp_len);
		(*p)->datalen = bef_cmp_len;
		memcpy((*p)->filename, tmp_filename, 24);
		memcpy((*p)->data, bef_cmp, bef_cmp_len);
		p = &(*p)->next;

		free(aft_enc); aft_enc=NULL;
		free(bef_cmp); bef_cmp=NULL;
		free(bef_enc); bef_enc=NULL;
		memset(tmp_filename, 0, 24);
		cmp = 0;
		aft_datalen = 0;
		dec_data_len = 0;
		bef_cmp_len = 0;
		filenumber--;
	}
	*p = NULL;
	fclose(res);
	return res_data_head;
}

static void release_res_file(struct res_data *res_data_head)
{
	struct res_data *tmp_p = NULL;
	while(res_data_head)
	{
		tmp_p = res_data_head->next;
		free(res_data_head);
		res_data_head = tmp_p;
	}
}

void sgn_native_handle_result(void *usr_data, int type, int size, void *data)
{
    cJSON *all = NULL, *json = NULL, *item=NULL;
    char  *str = NULL;
    struct sgn_native *native = (struct sgn_native *)usr_data;
    char *p = strrchr((const char*)data, '\n');
    if(p){
        if(p == data+(strlen((char *)data))-1){
            *p = '\0';
            size -= 1;
        }
    }
    if ((0 < size) && (NULL != data) &&  native->event->result_not_returned>0)
    {
        if(type == SKEGN_MESSAGE_TYPE_JSON){
        	native->event->result_not_returned--;
            all = cJSON_CreateObject();
            cJSON_AddStringToObject(all, "version", SKEGN_VERSION);
            cJSON_AddNumberToObject(all, "eof", 1);
            cJSON_AddStringToObject(all, "tokenId", native->event->callback[1].token_id);
            json = cJSON_Parse(data);
            if(json){
                if((item=cJSON_GetObjectItem(json, "error_msg")) != NULL){
                    cJSON_AddNumberToObject(all, "errId", 20015);
                    cJSON_AddStringToObject(all, "error", item->valuestring);
                    cJSON_Delete(json);
                }else{
                    cJSON_AddItemToObject(all, "result", json);
                }
            }
            str = cJSON_PrintUnformatted(all);
            native->event->callback[1].callback(native->event->callback[1].user_data, native->event->callback[1].token_id, SKEGN_MESSAGE_TYPE_JSON, str, strlen(str)+1);
        }else{
            native->event->callback[1].callback(native->event->callback[1].user_data, native->event->callback[1].token_id, SKEGN_MESSAGE_TYPE_JSON, data, size+1);
        }
    }
    if(str != NULL){
        free(str);
    }
    if(all){
        cJSON_Delete(all);
    }
}

static void _native_handle_err(struct sgn_native *native, int err_id, char *err_info)
{
    cJSON *all = NULL;
    char  *str = NULL;

    if(native->event->result_not_returned>0){
        native->event->result_not_returned--;
        all = cJSON_CreateObject();
        cJSON_AddNumberToObject(all, "errId", err_id);
        cJSON_AddNumberToObject(all, "eof", 1);
        cJSON_AddStringToObject(all, "error", err_info);
        cJSON_AddStringToObject(all, "tokenId", native->event->callback[1].token_id);
        str = cJSON_PrintUnformatted(all);

        if(str != NULL){
            native->event->callback[1].callback(native->event->callback[1].user_data, native->event->callback[1].token_id, SKEGN_MESSAGE_TYPE_JSON, str, strlen(str)+1);
            free(str);
        }
        if(all){
            cJSON_Delete(all);
        }
    }
}


static int
_count_module() {
    return sizeof(SGN_NATIVE_MODULES)/sizeof(SGN_NATIVE_MODULES[0]) - 1;
}


struct sgn_native *sgn_native_new(void *event, const char *strcfg)
{
    int rv = 0;
    int i = 0;
    cJSON *jsonobj;

    struct sgn_native *native = NULL;
    struct res_data *res_data_head = NULL;
   
    if(res_init_flag == 0)
    {
        if(strcfg != NULL){
            res_data_head = decode_resfile(strcfg);
            if(NULL == res_data_head)
            {
                goto end;
            }
            InitResource2(res_data_head);
            release_res_file(res_data_head);
        }else{
            InitResource2(NULL);
        }
        res_init_flag = 1;
    }else
    {
        res_init_flag++;
    }
    

    native = calloc(1, sizeof(*native) + _count_module() * sizeof(void *));
    if (!native) {
        goto end;
    }
    native->callback.callback = sgn_native_handle_result;
    native->callback.user_data = native;
    native->event = (event_t *)event;
#ifdef USE_NATIVE_VAD
    native->vad = sgn_native_vad_new(NULL);
#endif
	for( ;i<_count_module(); i++){
		native->instances[i] = SGN_NATIVE_MODULES[i].new(strcfg);
		if (native->instances[i] == NULL) {
			rv = -1;
			goto end;
		}
	}

end:
    if (rv && native) {
        sgn_native_del(native);
        native = NULL;
    }

    return native;
}


int
sgn_native_del(struct sgn_native *native)
{
    int i, n;

    n = _count_module();
    for (i = 0; i < n; i++) {
        if (native->instances[i]) {
            SGN_NATIVE_MODULES[i].delete(native->instances[i]);
        }
    }
    free(native);
    if(res_init_flag == 1)
    {
    	DestroyResource();
    	res_init_flag = 0;
    }
    else
    {
    	res_init_flag--;
    }
    return 0;
}

int sgn_native_start(struct sgn_native *native, const char *param, int vad)
{
    int rv = -1;
    char *request_param = NULL;
    char *error = NULL;
    char *vad_para = NULL;
    cJSON *jsonobj = NULL, *jsonitem = NULL;
    cJSON *item = NULL;
//    native->callback.callback = callback;
    jsonobj = cJSON_Parse(param);
    if (!jsonobj) {
        _native_handle_err(native, 20000, "cmd must be json format.");
        goto end;
    }
#ifdef USE_NATIVE_VAD
    if(vad){
        jsonitem = cJSON_GetObjectItem(jsonobj, "vad");
        vad_para = cJSON_PrintUnformatted(jsonitem);
        sgn_native_vad_start(native->vad, vad_para, &(native->callback));
        if(vad_para)free(vad_para);
        return 0;
    }
#endif
#ifdef USE_NATIVE
    if(native->event->cfg->native_enable && sgn_auth_verify(native->event->cfg->provision, native->event->cfg->app_key, native->event->cfg->secret_key, native->event->param->core_type, &error) != 0){
        // printf("auth check native\n");fflush(stdout);
        _native_handle_err(native, 20014, error);
        goto end;
    }
#endif

    jsonitem = cJSON_GetObjectItem(jsonobj, "request");
    request_param = cJSON_PrintUnformatted(jsonitem);

    jsonitem = cJSON_GetObjectItem(jsonitem, "coreType");
    if (jsonitem == NULL || !native->instances) {
        _native_handle_err(native, 20006, "param:request:coreType is invalid.");
        goto end;
    }

#ifdef USE_NATIVE_ALI
    if(strcmp(jsonitem->valuestring,"align.eval")==0){
    	native->i = SKEGN_NATIVE_ALI;
    }else
#endif
#ifdef USE_NATIVE_OPEN
    if(strcmp(jsonitem->valuestring,"open.eval")==0){
    	native->i = SKEGN_NATIVE_OPEN;
    }else
#endif
    if(strstr(jsonitem->valuestring,".eval")){
    	native->i = 0;
    }
#ifdef USE_NATIVE_REC
    else if (strstr(jsonitem->valuestring,".rec")){
    	native->i = SKEGN_NATIVE_REC;
    }
#endif
    /* 检查是否保存音频文件 */ //TODO: 是否存在native->sv start&feed同步问题, 只有非合成内核才保存音频
//    if(strstr(param, "saveAudio"))
//    {
//        if(NULL == native->sv)
//        {
//            native->sv = (sgn_save_s *)calloc(1, sizeof(sgn_save_s));
//            if(NULL == native->sv)goto end;
//        }
//        if(0 != sgn_save_start(native->sv, cJSON_GetObjectItem(jsonobj, "audio"), native->cur_tokenid))
//        {
//            free(native->sv);
//            native->sv = NULL;
//            _native_handle_err(native, 20008, "param:audio:saveAudio param is invalid.");
//            goto end;
//        }
//    }
//    else
//    {
//        if(NULL != native->sv)  /* 若之前的请求要求save音频但当前请求不save音频则释放sv */
//        {
//            sgn_save_del(native->sv);
//            native->sv = NULL;
//        }
//    }

    /*  每次start本地内核前reset一次内核  */
    //    sgn_native_cancel(native);

    rv = SGN_NATIVE_MODULES[native->i].start(native->instances[native->i], request_param, &(native->callback));
    if (0 != rv)
    {
        _native_handle_err(native, 20007, "param is invalid.");
    }

end:
    if (request_param) {
        free(request_param);
    }

    if (jsonobj) {
        cJSON_Delete(jsonobj);
    }
    return rv;
}

int
sgn_native_feed(struct sgn_native *native, const void *data, int size, int vad)
{
    int rv = -1;
#ifdef USE_NATIVE_VAD
    if(vad){
        sgn_native_vad_feed(native->vad, data, size);
        return 0;
    }
#endif
    if (native->i < 0 || !native->instances) {
        goto end;
    }
    /* 保存音频数据 */
//    if(NULL != native->sv)
//    {
//        sgn_save_append(native->sv, data, size);
//    }

    rv =  SGN_NATIVE_MODULES[native->i].feed(native->instances[native->i], data, size);

end:
    return rv;
}

int
sgn_native_stop(struct sgn_native *native, int vad)
{
    int rv   = -1;
#ifdef USE_NATIVE_VAD
    if(vad){
        sgn_native_vad_stop(native->vad);
        return 0;
    }
#endif
    if (native->i < 0 || !native->instances)
    {
        goto end;
    }
//    if(NULL != native->sv)
//    {
//        sgn_save_finish(native->sv);
//    }

    rv = SGN_NATIVE_MODULES[native->i].stop(native->instances[native->i]);

end:
    return rv;
}

int
sgn_native_cancel(void *nt)
{
    struct sgn_native *native = (struct sgn_native *)nt;
    int rv = 0;

    if (native->i < 0 || !native->instances)
    {
        return 0;
    }

    rv = SGN_NATIVE_MODULES[native->i].cancel(native->instances[native->i]);

    return rv;
}


#endif
