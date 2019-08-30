/*
 * engine.c
 *
 *  Created on: 2018年8月21日
 *      Author: weicong.liu
 */

#include <zlib.h>
#include <string.h>
#include "sgn_engine.h"
#include "sgn_event.h"
#include "lib/sgn_dbg.h"
#include "platform/sgn_common.h"
#include "native/sgn_auth.h"

#include "third/cJSON/cJSON.h"
#include "third/xxtea/sgn_secure_code.h"

void static sgn_get_sdk_cfg(struct sgn_buf *sdk_cfg)
{
    int ret = -1;
    int len = 0;
    char sdk_cfg_path[1024] = {0};
    char enc_buf[4096] = {0};
    unsigned char comp = 0;
    unsigned long dec_data_len = 0, bef_cmp_len=0;
    unsigned char *bef_cmp=NULL;
    char *dec_buf = NULL;
    FILE *file = NULL;
#if defined __IPHONE_OS__ || defined __WIN32__
    sgn_get_app_path(sdk_cfg_path);
#elif defined __ANDROID__
    sgn_get_app_path(sdk_cfg_path, NULL, NULL);
#endif
    strcpy(sdk_cfg_path+strlen(sdk_cfg_path), "sdk.cfg");
    LOG(LOG_ERROR, "sdk_cfg_path --%s", sdk_cfg_path);
    if((file = fopen(sdk_cfg_path, "rb"))==NULL)goto end;
    if(fseek(file, 0, SEEK_END))goto end;
    len = ftell(file);
    if(fseek(file, 0, SEEK_SET))goto end;
    if(fread(enc_buf, 1, len, file)<len)goto end;
    memcpy(&comp, enc_buf, 1);
    dec_data_len = sgn_secure_code(enc_buf+1, len-1, " fuck u crack", 9, &dec_buf, 'd'); // 解密
    memcpy(enc_buf, dec_buf, dec_data_len);
    if(dec_buf){free(dec_buf);dec_buf=NULL;}
    bef_cmp_len = dec_data_len*comp;
    if((bef_cmp = malloc(bef_cmp_len))==NULL)goto end;
    memset(bef_cmp, 0, bef_cmp_len);
    if(uncompress(bef_cmp, &bef_cmp_len, enc_buf, dec_data_len) == 0)                               // 解压缩
    {
        sgn_buf_append_str(sdk_cfg, bef_cmp);
    }else{
        goto end;
    }
    ret = 0;
end:
    if(ret)
    {
        sgn_buf_append_str(sdk_cfg, DEFAULT_SDK_CFG);
    }
    if(file)fclose(file);
    if(bef_cmp)free(bef_cmp);
}

/*
 *  描述: 初始化配置数据结构
 *  参数: 引擎配置json字符串
 *  返回值: success 返回指针
 *          failed  返回NULL，只有s_cfg不符合参数要求或者内存不足才会failed
 *
 */
cfg_t *sgn_cfg_new(const char *s_cfg) {
    int ret = -1;
#ifdef USE_NATIVE
    char provision_path[1024] = {0};
#endif
    cJSON *item = NULL, *json = NULL, *j_cfg = NULL;
    cfg_t *cfg = NULL;
    if (s_cfg == NULL)goto end;
    j_cfg = cJSON_Parse(s_cfg);
    if (j_cfg == NULL)goto end;
    cfg = (cfg_t *) malloc(sizeof(*cfg));
    if (cfg == NULL)goto end;
    memset(cfg, 0, sizeof(*cfg));
    cfg->log_enable = 1;
    cfg->nk_log_enable = 1;
    cfg->ping_space = 20;             // 20 senconds
    cfg->connect_timeout = 20;         // 20 senconds
    cfg->server_timeout = 60;        // 60 senconds
    strncpy(cfg->server, DEFAULT_SERVER, SGN_MIN_LEN);

    if ((item = cJSON_GetObjectItem(j_cfg, "appKey")) != NULL && item->type == cJSON_String) {
        strncpy(cfg->app_key, item->valuestring, SGN_MIN_LEN);
    }else{
        goto end;
    }
    if ((item = cJSON_GetObjectItem(j_cfg, "secretKey")) != NULL && item->type == cJSON_String) {
        strncpy(cfg->secret_key, item->valuestring, SGN_MIN_LEN);
    }else{
        goto end;
    }

    if ((json = cJSON_GetObjectItem(j_cfg, "cloud")) != NULL) {
        if ((item = cJSON_GetObjectItem(json, "server")) != NULL && item->type == cJSON_String) {
            snprintf(cfg->server, SGN_MIN_LEN, "%s", item->valuestring);
        }
        if ((item = cJSON_GetObjectItem(json, "sdkCfgAddr")) != NULL && item->type == cJSON_String) {
            strncpy(cfg->sdk_cfg_url, item->valuestring, SGN_MIN_LEN);
        }
        if ((item = cJSON_GetObjectItem(json, "autoSwitchProtocol")) != NULL && item->type == cJSON_Number) {
            cfg->auto_switch_protocol = item->valueint;
        }
        if ((item = cJSON_GetObjectItem(json, "connectTimeout")) != NULL && item->type == cJSON_Number) {
            cfg->connect_timeout = item->valueint;
        }
        if ((item = cJSON_GetObjectItem(json, "serverTimeout")) != NULL && item->type == cJSON_Number) {
            cfg->server_timeout= item->valueint;
        }
        cfg->cloud_enable = 1;
    }
    if ((json = cJSON_GetObjectItem(j_cfg, "sdkLog")) != NULL){
        if ((item = cJSON_GetObjectItem(json, "enable")) != NULL && item->type == cJSON_Number) {
            if(item->valueint==1){
                if ((item = cJSON_GetObjectItem(json, "output")) != NULL && item->type == cJSON_String){
                    sgn_log_set_file((const char *)item->valuestring);
                }
            }
        }
    }
#ifdef USE_NATIVE
    if ((json = cJSON_GetObjectItem(j_cfg, "vad")) != NULL) {
        if ((item = cJSON_GetObjectItem(json, "enable")) != NULL && item->type == cJSON_Number) {
            cfg->vad_enable = item->valueint;
        }
    }
    if((item=cJSON_GetObjectItem(j_cfg, "native")) != NULL && item->type == cJSON_String) {
        cfg->res_path = sgn_buf_new();
        sgn_buf_append_str(cfg->res_path, item->valuestring);
        cfg->native_enable = 1;
    }
    if((item=cJSON_GetObjectItem(j_cfg, "localAuthAddress")) != NULL && item->type == cJSON_String) {
        strncpy(cfg->auth_addr, item->valuestring, SGN_MIN_LEN);
    }
    if((item=cJSON_GetObjectItem(j_cfg, "provision")) != NULL && item->type == cJSON_String && strlen(item->valuestring)>0) {
        cfg->provision_path = sgn_buf_new();
        sgn_buf_append_str(cfg->provision_path, item->valuestring);
    }
    if(cfg->native_enable){
        //一个目的：provision_path在provision初始化后是一个可写入的路径，由以下3步完成
        if(cfg->provision_path != NULL && access(cfg->provision_path->buf, F_OK|R_OK|W_OK) == 0){
            //1、传入的路径是可写的：直接初始化provision
            cfg->provision = sgn_provision_new(cfg->provision_path->buf);
        }else{
            #if defined __IPHONE_OS__
            sgn_get_app_path(provision_path);
            #elif defined __ANDROID__
            sgn_get_app_path(provision_path, NULL, NULL);
            #endif
            if(strlen(provision_path)>0 && access(provision_path, W_OK)!=0){
                LOG(LOG_ERROR, "The app path:%s can't be writen", provision_path);
                goto end;
            }
            strcpy(provision_path+strlen(provision_path), "skegn.provision");
            if(cfg->provision_path != NULL){
                //2、传入的是不可写的：初始化后路径改成新生成的可写路径，以备下载新证书后写入
                cfg->provision = sgn_provision_new(cfg->provision_path->buf);
                sgn_buf_reset(cfg->provision_path);
                sgn_buf_append_str(cfg->provision_path, provision_path);
            }else{
                //3、没传入：改成新生成的可写路径，用于初始化或者写入
                cfg->provision_path = sgn_buf_new();
                sgn_buf_append_str(cfg->provision_path, provision_path);
                cfg->provision = sgn_provision_new(cfg->provision_path->buf);
            }
        }
        sgn_provision_set_auth_addr(cfg->provision, cfg->auth_addr);  //todo ?
#if defined __ANDROID__
        sgn_get_device_id(cfg->device_id, NULL, NULL);
#else
        sgn_get_device_id(cfg->device_id);
#endif
    }
#endif
    if(cfg->native_enable==0 && cfg->cloud_enable==0){
        cfg->cloud_enable = 1;
    }
    if(cfg->cloud_enable){
        cfg->sdk_cfg = sgn_buf_new();
        sgn_get_sdk_cfg(cfg->sdk_cfg);
    }

    ret = 0;
end:
    if (ret) {
        if (cfg) {
            sgn_cfg_delete(cfg);
            cfg = NULL;
        }
    }
    if(j_cfg){
        cJSON_Delete(j_cfg);
    }
    return cfg;
}

void sgn_cfg_delete(cfg_t *cfg) {
    if (cfg){
        if(cfg->sdk_cfg)sgn_buf_delete(cfg->sdk_cfg);
#ifdef USE_NATIVE
        if(cfg->provision_path)sgn_buf_delete(cfg->provision_path);
        if(cfg->res_path)sgn_buf_delete(cfg->res_path);
        if(cfg->provision)sgn_provision_delete(cfg->provision);
#endif
        free(cfg);
    }
}
/*
 *  方法名: sgn_engine_new
 *  描述:   初始化引擎数据结构
 *  参数:   引擎配置，录音参数指针，事件指针
 *  返回值: success 返回引擎指针
 *          failed  返回NULL，只有skegn_new传入的s_cfg不符合参数要求或者内存不足才会failed
 *
 */

struct skegn *sgn_engine_new(cfg_t* cfg, event_t *event) {
    int ret = -1;
    struct skegn *engine = (struct skegn *) malloc(sizeof(*engine));
    if(engine == NULL)goto end;
    memset(engine, 0, sizeof(*engine));
    engine->cfg = cfg;
    engine->event = event;
    ret = 0;
end:
    if(ret){
        if(engine){
            sgn_engine_delete(engine);
            engine = NULL;
        }
    }
    return engine;
}

void sgn_engine_delete(struct skegn *engine) {
    if (engine) {
        if (engine->event) {
            sgn_event_delete(engine->event);
            engine->event = NULL;
        }
        if (engine->cfg) {
            sgn_cfg_delete(engine->cfg);
            engine->cfg = NULL;
        }
        free(engine);
    }
}

