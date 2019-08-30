/*
 * sgn_event.c
 *
 *  Created on: 2018年8月21日
 *      Author: weicong.liu
 */

#include <pthread.h>

#include "skegn.h"
#include "sgn_engine.h"
#include "platform/sgn_common.h"
#include "lib/sgn_buf.h"
#include "lib/sgn_msg_queue.h"
#include "lib/sgn_dbg.h"
#include "lib/sgn_sha1.h"
#include "lib/sgn_hmac.h"
#include "lib/sgn_sound_intensity.h"
#include "native/sgn_auth.h"
#include "native/sgn_native_event.h"

#include "third/cJSON/cJSON.h"
#include "third/mongoose/mongoose.h"
#include "third/xxtea/sgn_secure_code.h"

static void reset_status(event_t *event)
{
//    event->http_connect_to_do = 0;
//    event->is_connected = 0;
//    event->is_result_returned = 0;
//    event->is_hanshake = 0;
//    event->ws_connect_status = 0;
}

static struct nk_info
{
    int err_id;
    char *error;
    char token_id[64];
};

static void nk_log(event_t *event, int eid, int est, struct nk_info *info)
{
    char body[4096] = {0};
    char buf[10240] = {0};
    char user_id[64] = {0};
    msg_t *msg = NULL;
    long long timestamp;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    timestamp = tv.tv_sec;
    timestamp = timestamp*1000 + tv.tv_usec/1000;
    if(strlen(event->param->user_id) > 0){
        strncpy(user_id, event->param->user_id, 63);
    }else{
        strcpy(user_id, "comm_sdk");
    }
    if(eid == 3 && est == 102)
    {
         sprintf(body, "log={\"body\":{\"errid\":%d,\"error\":%s,\"tokenId\":%s}}",\
                info->err_id, info->error, info->token_id);
        
    }else{
        sprintf(body, "log={\"body\":{\"prot\":1,\"source\":%d,\"version\":%d,\"timestamp\":%lld}}",\
                SOURCE, sgn_get_full_version(), timestamp);
     }
    sprintf(buf, "?eid=%d&est=%d&applicationId=%s&uid=%s %s",\
            eid, est, event->cfg->app_key, user_id, body);
    msg = sgn_new_msg(SKEGN_HTTP_LOG_TYPE, buf, strlen(buf)+1, NULL);
    sgn_queue_push(&event->http_post_log_queue, msg);
}

static void sgn_cloud_callback(event_t *event, const char *data, int data_size, int is_encoded)
{
    char *buf = NULL;
    static int ret = -1;
    if(is_encoded && event->ws_enc==3){
        data_size = sgn_secure_code((char *)data, data_size, event->ws_decrypt_key, sizeof(event->ws_decrypt_key), &buf, 'd');
        if(data_size==0)goto end;
        buf = realloc(buf, data_size+1);
        if(buf==NULL)goto end;
        *(buf+data_size) = '\0';
    }else{
        buf = (char *)calloc(1, data_size+1);
        if(buf==NULL)goto end;
        memcpy(buf, data, data_size);
    }
    ret = 0;
end:
    if(ret){
        char tmp[1024] = {0};
        sprintf(tmp, "{\"errId\":20016,\"eof\":1,\"error\":\"No enough memory\",\"tokenId\":\"%s\"}", event->callback[event->server_type].token_id);
        event->callback[event->server_type].callback(event->callback[event->server_type].user_data, event->callback[event->server_type].token_id, SKEGN_MESSAGE_TYPE_JSON, tmp, strlen(tmp));
    }else{
        event->callback[event->server_type].callback(event->callback[event->server_type].user_data, event->callback[event->server_type].token_id, SKEGN_MESSAGE_TYPE_JSON, buf, data_size);
        if(buf){free(buf);}
    }
    event->result_not_returned--;
    event->stop_time_stamp = 0;
}

static int sgn_encrypt_msg(event_t *event,  int type)
{
    char *encrypt_data = NULL;
    cJSON *msg_json   = NULL;
    cJSON *param_json = NULL;
    cJSON *app_json   = NULL;
    cJSON *tmp_json   = NULL;
    int len = 0;
    char appkey[64] = {0};
    char timestamp[64] = {0};
    char pre_encode[128] = {0};
    struct sha1_context sha1;
    if(event->ws_enc==0){
        DBG("enc ======== 0");
        return 0;
    }
    if(1 == type)   // connect msg
    {
        *(event->send_buf->buf+event->send_buf->data_len) = '\0';
        msg_json = cJSON_Parse(event->send_buf->buf);
        if(NULL != msg_json)
        {
            param_json = cJSON_GetObjectItem(msg_json, "param");
            if(NULL != param_json)
            {
                app_json = cJSON_GetObjectItem(param_json, "app");
                if(NULL != app_json)
                {
                    tmp_json = cJSON_GetObjectItem(app_json, "applicationId");
                    if(NULL != tmp_json)
                    {
                        strcpy(appkey, tmp_json->valuestring);
                    }
                    tmp_json = cJSON_GetObjectItem(app_json, "timestamp");
                    if(NULL != tmp_json)
                    {
                        strcpy(timestamp, tmp_json->valuestring);
                    }
                    sprintf(pre_encode, "%s%ssalt", appkey, timestamp);
                    DBG("pre_encode :%s", pre_encode);
                }
            }
            cJSON_Delete(msg_json);
        }
        len = sgn_secure_code(event->send_buf->buf, event->send_buf->data_len, event->ws_encrypt_key, 20, &encrypt_data, 'e');
        sgn_sha1_init(&sha1);
        sgn_sha1_update(&sha1, (unsigned char *)pre_encode, strlen(pre_encode));
        sgn_sha1_final(&sha1, event->ws_encrypt_key);
    }
    else
    {
        len = sgn_secure_code(event->send_buf->buf, event->send_buf->data_len, event->ws_encrypt_key, 20, &encrypt_data, 'e');
    }
    sgn_buf_reset(event->send_buf);
    if(encrypt_data){
        sgn_buf_append(event->send_buf, encrypt_data, len);
        free(encrypt_data);
    }
    return len;
}

static int package_connect(event_t *event)
{
    int ret = -1;
    char tmp[512] = {0};
    char timestamp[64] = {0};
    char sig[64] = {0};
    char *str = NULL;
    cJSON *all=NULL, *json=NULL, *item=NULL;
    all = cJSON_CreateObject();
    cJSON_AddStringToObject(all, "cmd", "connect");
    json = cJSON_CreateObject();
#ifdef __IPHONE_OS__
    static sgn_system_info_t system_info = {0};
#else
    static sgn_system_info_t system_info = {0};
#endif
    if(0==system_info.is_get)
    {
         sgn_get_system_info(&system_info);
        system_info.is_get = 1;
    }
    system_info.protocol = event->protocol;
    sprintf(tmp, "{\"version\": %d,\"source\":%d,\"arch\":\"%s\",\"protocol\":%d,\"os\":\"%s\",\"os_version\":\"%s\",\"product\":\"%s\"}",
                      system_info.version, system_info.source, system_info.arch,
                      system_info.protocol, system_info.os, system_info.os_version, system_info.product);
    item = cJSON_Parse(tmp);
    if(NULL != item)
    {
        cJSON_AddItemToObject(json, "sdk", item);
    }

    sprintf(timestamp, "%ld", (unsigned long)time(NULL));
    sprintf(tmp, "%s%s%s", event->cfg->app_key, timestamp, event->cfg->secret_key);
    sgn_sha1(tmp, strlen(tmp), sig);
    sprintf(tmp, "{\"applicationId\":\"%s\",\"timestamp\":\"%s\",\"sig\":\"%s\"}", event->cfg->app_key, timestamp, sig);
    item = cJSON_Parse(tmp);
    if(NULL != item)
    {
       cJSON_AddItemToObject(json, "app", item);
    }
    cJSON_AddItemToObject(all, "param", json);
    str = cJSON_PrintUnformatted(all);
    sgn_buf_append(event->send_buf, str, strlen(str));
    free(str);
    cJSON_Delete(all);
    ret = 0;
    return ret;
}

static int package_start(msg_t *msg, event_t *event)
{
    int ret = 0;
    char *str = NULL;
    cJSON *all=NULL, *json=NULL, *j_param=NULL;
	
    all = cJSON_CreateObject();
    cJSON_AddStringToObject(all, "cmd", "start");
    sprintf(timestamp, "%ld", (unsigned long)time(NULL));
    if(event->protocol==WEBSOCKET_PROTOCOL){
        sprintf(tmp, "%s%s%s", event->cfg->app_key, timestamp,  event->cfg->secret_key);
    }else{
        sprintf(tmp, "%s%s%s%s", event->cfg->app_key, timestamp, event->param->user_id, event->cfg->secret_key);
    }
    sgn_sha1(tmp, strlen(tmp), sig);
    j_param = cJSON_Parse((const char *)msg->data);
    json = cJSON_GetObjectItem(j_param, "app");
    cJSON_AddStringToObject(json, "applicationId", event->cfg->app_key);
    cJSON_AddStringToObject(json, "timestamp", timestamp);
    cJSON_AddStringToObject(json, "sig", sig);

    json = cJSON_GetObjectItem(j_param, "request");
    cJSON_AddStringToObject(json, "tokenId", event->callback[0].token_id);
    if(strcmp(event->param->compress, "opus") == 0){
        json = cJSON_GetObjectItem(j_param, "audio");
        cJSON_AddStringToObject(json, "audioType", "opus");
    }

    cJSON_AddItemToObject(all, "param", j_param);
    str = cJSON_PrintUnformatted(all);
    sgn_buf_append(event->send_buf, str, strlen(str));
LOG(LOG_ERROR,)
    free(str);
    cJSON_Delete(all);
    return ret;
}

static void handle_start_msg(struct mg_connection *nc, msg_t *msg, int protocol, event_t *event)
{
    if(event->ws_connect_status < NET_CONNECTED){
        DBG("--MG_EV_POLL- connect-----------opt");
        sgn_buf_reset(event->send_buf);
        package_connect(event);
        sgn_encrypt_msg(event, 1);
        mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, event->send_buf->buf, event->send_buf->data_len);
        sgn_buf_reset(event->send_buf);
        event->ws_connect_status = NET_CONNECTED;
    }
    DBG("--MG_EV_POLL- start-----------opt");
    package_start(msg, event);
    sgn_encrypt_msg(event, 0);
    mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, event->send_buf->buf, event->send_buf->data_len);
    sgn_buf_reset(event->send_buf);

    if(strcmp(event->param->compress, "opus") == 0){
        if(event->opus_encode == NULL){
            event->opus_encode = sgn_opus_encode_new(16000);
        }
        sgn_opus_encode_start(event->opus_encode, event->send_buf);
        if(event->send_buf->data_len > 0){
            mg_send_websocket_frame(nc, WEBSOCKET_OP_BINARY, event->send_buf->buf, event->send_buf->data_len);
            sgn_buf_reset(event->send_buf);
        }
    }
}

static void cloud_handle_msg(struct mg_connection *nc, msg_t *msg, int protocol, event_t *event)
{
    switch(msg->type){
        case SKEGN_START_TYPE:
            event->stop_time_stamp = 0;
            handle_start_msg(nc, msg, protocol, event);
            break;
        case SKEGN_FEED_TYPE:
            if(strcmp(event->param->compress, "opus") == 0){
                if(event->opus_encode == NULL){
                    event->opus_encode = sgn_opus_encode_new(16000);
                    sgn_opus_encode_start(event->opus_encode, event->send_buf);
                }
                sgn_opus_encode_append(event->opus_encode, msg->data, msg->data_len, 0, event->send_buf);
                if(event->send_buf->data_len > 0){
                    mg_send_websocket_frame(nc, WEBSOCKET_OP_BINARY, event->send_buf->buf, event->send_buf->data_len);
                    sgn_buf_reset(event->send_buf);
                }
            }else{
                mg_send_websocket_frame(nc, WEBSOCKET_OP_BINARY, msg->data, msg->data_len);
            }
            break;
        case SKEGN_STOP_TYPE:
            DBG("--MG_EV_POLL- stop-----------opt");
            if(strcmp(event->param->compress, "opus") == 0){
                sgn_opus_encode_append(event->opus_encode, msg->data, msg->data_len, 1, event->send_buf);
                if(event->send_buf->data_len > 0){
                    mg_send_websocket_frame(nc, WEBSOCKET_OP_BINARY, event->send_buf->buf, event->send_buf->data_len);
                    sgn_buf_reset(event->send_buf);
                }
            }
            mg_send_websocket_frame(nc, WEBSOCKET_OP_CONTINUE, "", 0);
            event->stop_time_stamp = time(NULL);
            break;
        case SKEGN_CANCEL_TYPE:
            DBG("--MG_EV_POLL- cancel-----------opt");
            event->stop_time_stamp = 0;
            mg_send_websocket_frame(nc, WEBSOCKET_OP_CLOSE, "", 0);
            nc->flags &= ~MG_F_USER_1;
            event->ws_connect_status = NET_BREAK;
            event->ws_connect_to_do = WS_TO_CONNECT;
            break;
    }
}


static void handle_err_msg(event_t *event, int err_id, const char *err_info)
{
    char *str = NULL;
    cJSON *all = NULL;
    DBG("errId:%d, info:%s", err_id, err_info);
    event->stop_time_stamp = 0;
    if(err_id==20009  && event->protocol==WEBSOCKET_PROTOCOL){
        event->ws_connect_status = NET_BREAK;
        if(event->ws_20009<1){
            event->ws_20009++;
            DBG("switch------event->cfg->auto_switch_protocol%d--", event->cfg->auto_switch_protocol);
            if(event->cfg!=NULL && event->cfg->auto_switch_protocol==1){
                DBG("switch--------");
                event->http_post_queue.next = event->ws_msg_queue.next;
                event->ws_msg_queue.next = NULL;
                event->protocol = HTTP_PROTOCOL;
            }else{
                event->ws_connect_to_do = WS_TO_RECONNECT_ALL;
            }
            return;
        }
    }
    sgn_queue_delete(&event->ws_msg_queue);
    DBG("event->result_not_returned:%d", event->result_not_returned);
    if(event->result_not_returned>0){
        struct nk_info info;
        memset(&info,0,sizeof(info));
        info.err_id = err_id;
        info.error = err_info;
        strcpy(info.token_id,event->callback[event->server_type].token_id);
        nk_log(event, 3, 102, &info); 
        
        all = cJSON_CreateObject();
        cJSON_AddNumberToObject(all, "errId", err_id);
        cJSON_AddNumberToObject(all, "eof", 1);
        cJSON_AddStringToObject(all, "error", err_info);
        cJSON_AddStringToObject(all, "tokenId", event->callback[event->server_type].token_id);
        str = cJSON_PrintUnformatted(all);
        DBG("result:%s", str);
        if(str != NULL && event->callback[event->server_type].callback!=NULL){
            sgn_cloud_callback(event, str, strlen(str), 0);
            free(str);
        }else{
            DBG("str==NULL or callback==NULL");
        }
        if(all){
            cJSON_Delete(all);
        }
    }
}

static void cloud_handle_err_msg(event_t *event, int err_id, const char *err_info, int protocol)
{
    if (event->protocol == protocol)
    {
        handle_err_msg(event, err_id, err_info);
    }
}

static void ws_ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
    struct websocket_message *wm = (struct websocket_message *) ev_data;
    (void) nc;
    event_t *event = nc->user_data;
    msg_t *msg = NULL;
    char tmp_con_str[SGN_MIN_LEN] = {0};
    // if (event->stop_time_stamp > 0) DBG("event->stop_time:%d, event->cfg->server_timeout:%d", event->stop_time_stamp, event->cfg->server_timeout);
    if(event->protocol==WEBSOCKET_PROTOCOL
            && event->result_not_returned>0
            && event->stop_time_stamp>0
            && (time(NULL)-event->stop_time_stamp)>event->cfg->server_timeout){
        LOG(LOG_ERROR, "event->stop_time:%d, event->cfg->server_timeout:%d", event->stop_time_stamp, event->cfg->server_timeout);
        cloud_handle_err_msg(event, 20013, "Server timeout.", WEBSOCKET_PROTOCOL);
    }
    switch (ev) {
        case MG_EV_CONNECT: {
            int status = *((int *) ev_data);
            DBG("--MG_EV_CONNECT- status:%d, nc->err:%d, event->server_count:%d", status, nc->err, event->ws_server_count);
            if (status != 0 || nc->err!=0) {
                event->ws_server_count--;
                if(event->ws_server_count==0){
                    LOG(LOG_ERROR, "error 20009, connect failed");
                    cloud_handle_err_msg(event, 20009, "Network abnormal.", WEBSOCKET_PROTOCOL);
                    nk_log(event, 1, 4, NULL);  
                    event->ws_connect_status = NET_BREAK;
                }
            }else{
            	LOG(LOG_ERROR, "MG_EV_CONNECT Succeed");
                nk_log(event, 1, 6, NULL);
            }
            break;
        }
        case MG_EV_WEBSOCKET_HANDSHAKE_DONE: {
            struct http_message *hm = (struct http_message *) ev_data;
            struct mg_str *accept = NULL;
            char key_seed[256] = {0};
            struct sha1_context sha1;
            DBG("--MG_EV_WEBSOCKET_HANDSHAKE_DONE- strlen(event->ws_con_str):%d", strlen(event->ws_addr_str));
            if (hm->resp_code == 101) {
                if(event->ws_connect_status < NET_HANDKSHAKE){
                    event->ws_connect_status = NET_HANDKSHAKE;
                    mg_sock_addr_to_str(&nc->sa, event->ws_addr_str, SGN_MIN_LEN, MG_SOCK_STRINGIFY_IP|MG_SOCK_STRINGIFY_PORT);
                    nc->flags |= MG_F_USER_1;       // MG_F_USER_1 mean select this nc to communicate with server
                    nk_log(event, 1, 3, NULL);            // 建立ws链接成功
                    //生成2个加密key
                    sgn_sha1_init(&sha1);
                    sprintf(key_seed, "%ssalt", nc->key);
                    sgn_sha1_update(&sha1, key_seed, strlen(key_seed));
                    sgn_sha1_final(&sha1, event->ws_encrypt_key);
                    accept = mg_get_http_header(hm, "Sec-WebSocket-Accept");
                    if(accept){
                        sgn_sha1_init(&sha1);
                        memset(key_seed, 0 ,sizeof(key_seed));
                        memcpy(key_seed, accept->p, accept->len);
                        strcpy(key_seed+accept->len, "salt");
                        sgn_sha1_update(&sha1, key_seed, strlen(key_seed));
                        sgn_sha1_final(&sha1, event->ws_decrypt_key);
                        event->ws_enc = 3;
                    }
                }else{
                    mg_send_websocket_frame(nc, WEBSOCKET_OP_CLOSE, "", 0);
                }
            }else{
                // nc->flags |= MG_F_USER_1;
                cloud_handle_err_msg(event, 20009, "Network abnormal.", WEBSOCKET_PROTOCOL);
                LOG(LOG_ERROR, "handshake error");
            }
            break;
        }
        case MG_EV_POLL: {
            if(nc->flags&MG_F_USER_1){
                if(strlen(event->param->core_type) > 0 && strcmp(event->ws_cur_core_type, event->param->core_type) != 0){
                    event->ws_connect_to_do = WS_TO_CHANGE_CORE_TYPE;
//                    event->ws_connect_status = NET_BREAK;
                    DBG("event->param->core_type:%s, event->cur_core_type:%s", event->param->core_type, event->ws_cur_core_type);
                }
                if(event->ws_connect_to_do==WS_TO_CHANGE_CORE_TYPE){
                    DBG("--MG_EV_POLL- change coreType event->ws_connect_to_do:%d", event->ws_connect_to_do);
                    mg_send_websocket_frame(nc, WEBSOCKET_OP_CLOSE, "", 0);
                    nc->flags &= ~MG_F_USER_1;
                }else{
                    for(msg = sgn_queue_pop(&event->ws_msg_queue); msg!=NULL; msg = sgn_queue_pop(&event->ws_msg_queue)){
                        cloud_handle_msg(nc, msg, WEBSOCKET_PROTOCOL, event);
                        free(msg);
                        if(event->ws_connect_status<NET_HANDKSHAKE)break;
                    }
                }
            }

            if(event->protocol == WEBSOCKET_PROTOCOL
                    && event->ws_connect_status < NET_HANDKSHAKE
                    && event->ws_connect_to_do==0
                    && time(NULL)-event->connect_time_stamp > event->cfg->connect_timeout)
            {
                LOG(LOG_ERROR, "error 20009, connect timeout");
                cloud_handle_err_msg(event, 20009, "Network abnormal.", WEBSOCKET_PROTOCOL);
            }
//            if(event->last_pong_time_stamp>0 && time(NULL)-event->last_pong_time_stamp > MG_WEBSOCKET_PING_INTERVAL_SECONDS){
//                DBG("--MG_WEBSOCKET_PING_INTERVAL_SECONDS-");
//                handle_err_msg(event, 20009, "Network abnormal.");
//                event->last_pong_time_stamp = 0;
//                event->nc = NULL;
//            }
            break;
        }
        case MG_EV_WEBSOCKET_CONTROL_FRAME:{
            DBG("MG_EV_WEBSOCKET_CONTROL_FRAME flags:%d", wm->flags&0xA);
            if((nc->flags & MG_F_USER_1) && (wm->flags&0xA) == 0xA){
                event->last_pong_time_stamp = time(NULL);
            }
            break;
        }
        case MG_EV_WEBSOCKET_FRAME: {
            if((nc->flags & MG_F_USER_1) && event->result_not_returned>0){
                sgn_cloud_callback(event, wm->data, wm->size, 1);
            }
            break;
        }
        case MG_EV_CLOSE: {
            DBG("--MG_EV_CLOSE-");
            if(nc->flags & MG_F_USER_1){
                LOG(LOG_ERROR, "error 20009, connection break");
                cloud_handle_err_msg(event, 20009, "Network abnormal.", WEBSOCKET_PROTOCOL);
                event->ws_connect_status = NET_BREAK;
                nk_log(event, 1, 5, NULL);
                // event->ws_connect_to_do = WS_TO_RECONNECT;
                DBG("--is_ws_break_error-");
            }
            break;
        }
    }

}

// sdk.cfg 下载处理
static void http_sdk_cfg_ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
    struct http_message *hm = (struct http_message *) ev_data;
    char sdk_cfg_path[1024] = {0};
    FILE *file = NULL;
      switch (ev) {
        case MG_EV_CONNECT:
          if (*(int *) ev_data != 0) {
              LOG(LOG_ERROR, "connect() failed: %s", strerror(*(int *) ev_data));
          }
          break;
        case MG_EV_HTTP_REPLY:
          nc->flags |= MG_F_CLOSE_IMMEDIATELY;
#if defined __IPHONE_OS__ || defined __WIN32__
          sgn_get_app_path(sdk_cfg_path);
#elif defined __ANDROID__
          sgn_get_app_path(sdk_cfg_path, NULL, NULL);
#endif
		 LOG(LOG_ERROR, "sgn_get_app_path : %s", sdk_cfg_path);
       
          strcpy(sdk_cfg_path+strlen(sdk_cfg_path), "sdk.cfg");
          if((file = fopen(sdk_cfg_path, "wb"))!=NULL){
              fwrite(hm->body.p, 1, hm->body.len, file);
              fclose(file);
          }
          break;
        case MG_EV_CLOSE:
            DBG("Server closed connection");
          break;
        default:
          break;
      }
}

// http post log 后处理
static void http_post_log_handler(struct mg_connection *nc, int ev, void *ev_data) {
    event_t *event = nc->user_data;
    switch (ev) {
        case MG_EV_CONNECT:
          if (*(int *) ev_data != 0) {
              LOG(LOG_ERROR, "connect() failed: %s", strerror(*(int *) ev_data));
          }else{
              if(strlen(nc->ip_addr))
                strcpy(event->log_ip_addr,nc->ip_addr);
          }
          break;
        case MG_EV_HTTP_REPLY:
            DBG("Post log success");
          break;
        case MG_EV_POLL:
          break;
        case MG_EV_CLOSE:
            DBG("Post log closed connection");
          break;
        default:
          break;
    }
}

// http post 后处理
static void http_post_handler(struct mg_connection *nc, int ev, void *ev_data) {
    event_t *event = nc->user_data;
    struct http_message *hm = (struct http_message *) ev_data;
    switch (ev) {
        case MG_EV_CONNECT:
            if (*(int *) ev_data != 0) {
                cloud_handle_err_msg(event, 20009, "Network abnormal.", HTTP_PROTOCOL);
                DBG("Post msg connect failed");
            }else{
//                event->http_connect_status = NET_CONNECTED;
                DBG("Post msg connected");
            }
            break;
        case MG_EV_HTTP_REPLY:
            if((nc->flags&MG_F_USER_2)==0){
                sgn_cloud_callback(event, hm->body.p, hm->body.len, 0);
                nk_log(event, 1, 10, NULL);
                sgn_buf_reset(event->http_send_buf);
                reset_status(event);
            }
            DBG("Post msg reply");
            break;
        case MG_EV_CLOSE:
            DBG("Post msg closed connection");
            break;
        default:
            break;
    }
}

param_t *sgn_param_new()
{
    int ret = -1;
    param_t *param = (param_t *)malloc(sizeof(*param));
    memset(param, 0, sizeof(*param));
    ret = 0;
end:
    if(ret){
        if(param){
            free(param);
            param = NULL;
        }
    }
    return param;
}

int start_event_task(event_t *event, msg_t *msg){

    int ret = -1;
    cJSON *j_param = NULL, *item = NULL, *json = NULL;
    DBG("");
    if(event == NULL)goto end;
    char *s_param = msg->data;
    memset(event->param, 0, sizeof(*event->param));
    j_param = cJSON_Parse(s_param);
    if(j_param == NULL){
        // todo:deal error
        goto end;
    }else{
        event->param->is_json = 1;
    }
    if((item = cJSON_GetObjectItem(j_param, "coreProvideType")) != NULL && item->type == cJSON_String) {
        strncpy(event->param->server_type, item->valuestring, SGN_MIN_MIN);
    }else{
        strncpy(event->param->server_type, "cloud", SGN_MIN_MIN);
    }
    event->protocol = 0;
    event->server_type = 0;
    if(strcmp(event->param->server_type, "cloud")==0){
        memcpy(&event->callback[0], msg->user_data, sizeof(callback_data_t));
    }else if(event->cfg->native_enable == 0){
        memcpy(&event->callback[0], msg->user_data, sizeof(callback_data_t));
        handle_err_msg(event, 20011, "No native module");
    }else{
        event->server_type = 1;
        memcpy(&event->callback[1], msg->user_data, sizeof(callback_data_t));
    }
    if(event->server_type == 0){
        event->protocol = WEBSOCKET_PROTOCOL;
        if((item = cJSON_GetObjectItem(j_param, "protocol")) != NULL && item->type == cJSON_String){
            if(strcmp(item->valuestring, "http")==0){
                event->protocol = HTTP_PROTOCOL;
            }
        }
    }
    if(event->cfg->vad_enable){
        memcpy(&event->callback[1], msg->user_data, sizeof(callback_data_t));
    }
    free(msg->user_data);
#ifndef USE_NATIVE
    if((item = cJSON_GetObjectItem(j_param, "soundIntensityEnable")) != NULL && item->type == cJSON_Number) {
        event->param->sound_intensity_enable = item->valueint;
    }
#endif

#ifdef USE_NATIVE
    if((item = cJSON_GetObjectItem(j_param, "serialNumber")) != NULL && item->type == cJSON_String) {
        strncpy(event->param->serial_number, item->valuestring, SGN_MIN_MIN);
    }
#endif
    if((json = cJSON_GetObjectItem(j_param, "request")) != NULL) {
        if((item=cJSON_GetObjectItem(json, "coreType")) != NULL && item->type == cJSON_String) {
            strncpy(event->param->core_type, item->valuestring, SGN_MIN_MIN);
        }
        event->param->has_request = 1;
    }

    if((json = cJSON_GetObjectItem(j_param, "app")) != NULL) {
        if((item=cJSON_GetObjectItem(json, "userId")) != NULL && item->type == cJSON_String) {
            strncpy(event->param->user_id, item->valuestring, SGN_MIN_MIN*2);
        }else{
            //todo: deal error
        }
    }
    if((json = cJSON_GetObjectItem(j_param, "audio")) != NULL) {
        if((item=cJSON_GetObjectItem(json, "audioType")) != NULL && item->type == cJSON_String) {
            strncpy(event->param->audio_type, item->valuestring, SGN_MIN_MIN);
        }else{
            strncpy(event->param->audio_type, "wav", SGN_MIN_MIN);
        }
        if(strcmp(event->param->audio_type, "wav")==0){
            if((item=cJSON_GetObjectItem(json, "compress")) != NULL && item->type == cJSON_String) {
                strncpy(event->param->compress, item->valuestring, SGN_MIN_MIN);
                if(strcmp(event->param->compress, "speex")==0){        // 兼容1.0.x传入speex也压缩成opus
                    strncpy(event->param->compress, "opus", SGN_MIN_MIN);
                }
            }else{
                strncpy(event->param->compress, "opus", SGN_MIN_MIN);
            }
        }
        if((item=cJSON_GetObjectItem(json, "sampleRate")) != NULL && item->type == cJSON_Number) {
            event->param->sample_rate = item->valueint;
        }else{
            event->param->sample_rate = 16000;
        }
        if((item=cJSON_GetObjectItem(json, "channel")) != NULL && item->type == cJSON_Number) {
            event->param->channel = item->valueint;
        }else{
            event->param->channel = 1;
        }
    }else{
        //todo :deal error
    }
    ret = 0;
end:
    if(j_param){
        cJSON_Delete(j_param);
    }
    return ret;
}

void sgn_param_delete(param_t *param)
{
    if(param)free(param);
}
// 检查 '参数'和'api调用步骤‘ 是否正确
int check_param_and_step(event_t *event, msg_t *msg)
{
    int ret = -1;
    char *error = NULL;
//    DBG("---------------------msg->type:%d, event->last_msg_type:%d\n", msg->type, event->last_msg_type);

    if(msg->type==SKEGN_START_TYPE){
        event->result_not_returned++;
        event->ws_20009 = 0;
        start_event_task(event, msg);
        if(event->last_msg_type!=SKEGN_DEFAUTL_TYPE && event->last_msg_type!=SKEGN_STOP_TYPE){
            handle_err_msg(event, 20010, "Interface calls in the wrong order");
            goto end;
        }
        if(strcmp(event->param->server_type, "cloud")==0 && event->ws_connect_status == NET_BREAK){
            event->ws_connect_to_do = WS_TO_CONNECT;
        }
    }
    if(event->result_not_returned == 0)goto end;

    if(msg->type==SKEGN_FEED_TYPE && event->last_msg_type!=SKEGN_START_TYPE && event->last_msg_type!=SKEGN_FEED_TYPE){
        handle_err_msg(event, 20010, "Interface calls in the wrong order");
        goto end;
    }
    if(msg->type==SKEGN_STOP_TYPE && event->last_msg_type!=SKEGN_START_TYPE && event->last_msg_type!=SKEGN_FEED_TYPE){
        handle_err_msg(event, 20010, "Interface calls in the wrong order");
        goto end;
    }

    if(msg->type==SKEGN_START_TYPE){
        //todo: check userId?
        if(event->param->is_json == 0){
            handle_err_msg(event, 20000, "Param is not json.");
            goto end;
        }
//        if(strlen(event->param->server_type) == 0){
//            //handle_err_msg(event, 2000x, "No serverType info in param.");
//            goto end;
//        }
        if(event->param->has_request==0){
            handle_err_msg(event, 20001, "No request info in param.");
            goto end;
        }
        if(strlen(event->param->core_type) == 0){
//            handle_err_msg(event, 2000x, "No coreType info in param.");??
            goto end;
        }
        if(strlen(event->param->audio_type) == 0){
            handle_err_msg(event, 20002, "No audio info in param.");
            goto end;
        }
        if(strcmp(event->param->audio_type, "wav")==0 && event->param->sample_rate!=16000){
            handle_err_msg(event, 20003, "The audio's sampleRate is invalid.");
            goto end;
        }
        if(strcmp(event->param->audio_type, "wav")==0 && event->param->channel!=1){
            handle_err_msg(event, 20004, "The audio's channel is invalid.");
            goto end;
        }
    }

    ret = 0;
end:
    if(ret){
        event->last_msg_type = SKEGN_DEFAUTL_TYPE;
    }else{
        event->last_msg_type = msg->type;
    }
    if(msg->type == SKEGN_CANCEL_TYPE){
        event->last_msg_type = SKEGN_DEFAUTL_TYPE;
    }
    return ret;
}

// pipe接收api接口的调用数据并压入消息堆栈msgqueue
static void pipe_ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
    struct mbuf *io = &nc->recv_mbuf;
    event_t *event = nc->user_data;
    msg_t *p_msg= NULL;
    char  buf[256] = {0};
#ifdef USE_NATIVE
    static time_t time_stamp = 0;
    static int provision_flag = -1;
#endif
    int copy_size = 0, bytes = 0;

    switch (ev) {
        case MG_EV_CONNECT:
            if (*(int *) ev_data != 0) {
               LOG(LOG_ERROR, "connect() failed: %s", strerror(*(int *) ev_data));
            }
            break;
        case MG_EV_RECV:
        case MG_EV_POLL:
#ifdef USE_NATIVE
        if(event->cfg->native_enable){
            // 如果证书失效 或  将在18小时内失效，等待最长4秒获取新证书，再评分
            if(time_stamp==0)time_stamp=time(NULL);
            if(provision_flag==-1)provision_flag=sgn_check_provision(event->cfg->provision);
            if(provision_flag<SGN_AUTH_EXPIRE_LESS_30D && time(NULL)-time_stamp<4){
                DBG("time_stamp:%d, provision_flag:%d", time_stamp, provision_flag);
                provision_flag = sgn_check_provision(event->cfg->provision);
                break;
            }
            if(provision_flag==SGN_AUTH_USE_CATFISH \
                && sgn_provision_get_catfish_auth(event->cfg->provision)==0 && time(NULL)-time_stamp<4){
                break;
            }
        }
        if(event->native==NULL){
            if (event->cfg->native_enable){
                event->native = sgn_native_new(event, event->cfg->res_path->buf);
            }else if(event->cfg->vad_enable){
                event->native = sgn_native_new(event, NULL);
            }
        }
#endif
            for(; (io->len>=sizeof(p_msg)) && event->ws_connect_to_do==0; mbuf_remove(io, sizeof(p_msg))){
                memcpy(&p_msg, io->buf, sizeof(p_msg));

                if(p_msg->type == SKEGN_DELETE_TYPE){  // stop event loop
                    event->is_running = 0;
                    free(p_msg);
                }else{
                    if(check_param_and_step(event, p_msg)==0){
#ifdef USE_NATIVE
                        if(strcmp(event->param->server_type, "cloud")==0){
#endif
                            if(p_msg->type == SKEGN_CANCEL_TYPE){
                                DBG("SKEGN_CANCEL_TYPE SKEGN_CANCEL_TYPE SKEGN_CANCEL_TYPE----------");
                                sgn_queue_delete(&event->ws_msg_queue);
                                sgn_queue_delete(&event->http_post_queue);
                            }
                            if(event->protocol==WEBSOCKET_PROTOCOL){
                                DBG("sgn queue push ws msg type %d", p_msg->type);
                                sgn_queue_push(&event->ws_msg_queue, p_msg);
                            }else{
                                DBG("sgn queue push http msg type %d", p_msg->type);
                                sgn_queue_push(&event->http_post_queue, p_msg);
                            }
#ifdef USE_NATIVE
                        }else/* if(strcmp(event->param->server_typ, "native")==0)*/{
                            native_handle_msg(p_msg, event, 0);
                        }
                        if(event->cfg->vad_enable){
                            native_handle_msg(p_msg, event, 1);
                        }
#else
                        if(event->param->sound_intensity_enable && event->last_msg_type == SKEGN_FEED_TYPE){
                            sprintf(buf, "{\"sound_intensity\": %f}", sgn_vad_sound_intensity(p_msg->data, p_msg->data_len));
                            event->callback[0].callback(event->callback[0].user_data, event->callback[0].token_id, SKEGN_MESSAGE_TYPE_JSON, buf, strlen(buf)+1);
                        }
#endif
                    }
                }
            }

            break;
        case MG_EV_CLOSE:
            event->pipe_is_ready = 0;
            break;
        default:
            break;
    }
}

// 检测是否下载sdk.cfg
static void add_check_download_sdk_cfg_event(event_t *event)
{
    time_t now = 0;
    char sdk_cfg_path[1024] = {0};
    struct stat buf;
    DBG("");
#if defined __IPHONE_OS__ || defined __WIN32__
    sgn_get_app_path(sdk_cfg_path);
#elif defined __ANDROID__
    sgn_get_app_path(sdk_cfg_path, NULL, NULL);
#endif
    strcpy(sdk_cfg_path+strlen(sdk_cfg_path), "sdk.cfg");
    now = time(NULL);
    if(stat(sdk_cfg_path, &buf) != 0 || (now-buf.st_mtime)>60*60){        //当文件不存在或者修改时间超过1个小时则下载cfg文件
		if(strlen(event->cfg->sdk_cfg_url) > 0)
		{
			mg_connect_http(&event->mgr, http_sdk_cfg_ev_handler, event->cfg->sdk_cfg_url, NULL, NULL);
		}else{
#ifdef USE_SSL
			mg_connect_http(&event->mgr, http_sdk_cfg_ev_handler, SECURE_SDK_CFG_ADDR, NULL, NULL);
#else	
			mg_connect_http(&event->mgr, http_sdk_cfg_ev_handler, DEFAULT_SDK_CFG_ADDR, NULL, NULL);
#endif
		}
    }
}

// 连接 服务集群
static void add_ws_connect_event(event_t *event)
{
    struct mg_connection *nc;
    char tmp[SGN_MIN_LEN] = { 0 };
    cJSON *json = NULL, *item = NULL, *value = NULL;
    int i = 0, j = 0;
    struct mg_connect_opts opts;
    DBG("");
    memset(&opts, 0, sizeof(opts));
    opts.user_data = event;
    event->ws_connect_status = NET_BREAK;
    if(strlen(event->param->core_type) > 0){
        strncpy(event->ws_cur_core_type, event->param->core_type, SGN_MIN_MIN);
    }
	LOG(LOG_ERROR,"WS_TO_RECONNECT_ALL : %d\n",event->ws_connect_to_do);
    if(event->ws_connect_to_do==WS_TO_RECONNECT_ALL
                && event->cfg->sdk_cfg!=NULL
                && (json = cJSON_Parse(event->cfg->sdk_cfg->buf)) != NULL
                && (item = cJSON_GetObjectItem(json, "serverList")) != NULL
                && item->type == cJSON_Array) {
            i = cJSON_GetArraySize(item);
            memset(event->ws_addr_str, 0, SGN_MIN_LEN);
            event->ws_server_count = 0;
            for (j = 0; j < i; j++) {
                value = cJSON_GetArrayItem(item, j);
				
                snprintf(tmp, SGN_MIN_LEN, "%s/%s?e=2&t=1", value->valuestring, event->ws_cur_core_type);
                LOG(LOG_ERROR,"connect 2:%s", tmp);
                nc = mg_connect_ws_opt(&event->mgr, ws_ev_handler, opts, tmp, "stkouyu", NULL);
                event->ws_server_count++;
            }
    }else{
    
        if(strlen(event->ws_addr_str) > 0){
            snprintf(tmp, SGN_MIN_LEN, "%s/%s?e=2&t=1", event->ws_addr_str, event->ws_cur_core_type);
			LOG(LOG_ERROR,"ws_addr_str : %s\n",tmp);
        }else{
            snprintf(tmp, SGN_MIN_LEN, "%s/%s?e=2&t=1", event->cfg->server, event->ws_cur_core_type);
			LOG(LOG_ERROR,"ws_addr_str : %s\n",tmp);
        }
        DBG("connect 1:%s", tmp);
        nc = mg_connect_ws_opt(&event->mgr, ws_ev_handler, opts, tmp, "stkouyu", NULL);
        event->ws_server_count = 1;
    }
    nk_log(event, 1, 7, NULL);        // 开始建立ws连接
    reset_status(event);
    event->connect_time_stamp = time(NULL);
    if (json != NULL) {
        cJSON_Delete(json);
    }
}

extern struct mg_connection *mg_connect_http_base(
        struct mg_mgr *mgr, MG_CB(mg_event_handler_t ev_handler, void *user_data),
        struct mg_connect_opts opts, const char *scheme1, const char *scheme2,
        const char *scheme_ssl1, const char *scheme_ssl2, const char *url,
        struct mg_str *path, struct mg_str *user_info, struct mg_str *host);
struct mg_connection *mg_connect_http_data_opt(
    struct mg_mgr *mgr, MG_CB(mg_event_handler_t ev_handler, void *user_data),
    struct mg_connect_opts opts, const char *url, const char *extra_headers,
    const char *post_data, int post_data_len) {
  struct mg_str user = MG_NULL_STR, null_str = MG_NULL_STR;
  struct mg_str host = MG_NULL_STR, path = MG_NULL_STR;
  struct mbuf auth;
  struct mg_connection *nc =
      mg_connect_http_base(mgr, MG_CB(ev_handler, user_data), opts, "http",
                           NULL, "https", NULL, url, &path, &user, &host);

  if (nc == NULL) {
    return NULL;
  }

  mbuf_init(&auth, 0);
  if (user.len > 0) {
    mg_basic_auth_header(user, null_str, &auth);
  }

  if (post_data == NULL) post_data = "";
  if (extra_headers == NULL) extra_headers = "";
  if (path.len == 0) path = mg_mk_str("/");
  if (host.len == 0) host = mg_mk_str("");

  mg_printf(nc, "%s %.*s HTTP/1.1\r\nHost: %.*s\r\nContent-Length: %" SIZE_T_FMT
                "\r\n%.*s%s\r\n",
            "POST", (int) path.len, path.p,
            (int) (path.p - host.p), host.p, post_data_len, (int) auth.len,
            (auth.buf == NULL ? "" : auth.buf), extra_headers);
  mg_send(nc, post_data, post_data_len);
  mbuf_free(&auth);
  return nc;
}

static void add_http_connect_event(event_t *event)
{
    msg_t *msg = NULL;
    char url[4096] = {0};
    char content_type[4096] = {0};
    for(msg = sgn_queue_pop(&event->http_post_queue); msg!=NULL; msg = sgn_queue_pop(&event->http_post_queue)){
        switch(msg->type){
            case SKEGN_START_TYPE:
                DBG("http post start");
                sgn_buf_reset(event->http_send_buf);
                sgn_buf_append_str(event->http_send_buf, "--");
                sgn_buf_append_str(event->http_send_buf, BOUNDARY);
                sgn_buf_append_str(event->http_send_buf, "Content-Disposition: form-data; name=\"text\"\r\nContent-Type: text/json\r\n\r\n");

                sgn_buf_append_str(event->http_send_buf, "{\"connect\":");
                sgn_buf_reset(event->send_buf);
                package_connect(event);
                sgn_buf_append(event->http_send_buf, event->send_buf->buf, event->send_buf->data_len);
                sgn_buf_reset(event->send_buf);

                sgn_buf_append_str(event->http_send_buf, ",\"start\":");
                package_start(msg, event);
                sgn_buf_append(event->http_send_buf, event->send_buf->buf, event->send_buf->data_len);
                sgn_buf_reset(event->send_buf);

                sgn_buf_append_str(event->http_send_buf, "}\r\n--");
                sgn_buf_append_str(event->http_send_buf, BOUNDARY);
                sgn_buf_append_str(event->http_send_buf, "\r\nContent-Disposition: form-data; name=\"audio\"\r\nContent-Type: audio/wav\r\n\r\n");
                if(strcmp(event->param->compress, "opus") == 0){
                    if(event->opus_encode == NULL){
                        event->opus_encode = sgn_opus_encode_new(16000);
                    }
                    sgn_buf_reset(event->send_buf);
                    sgn_opus_encode_start(event->opus_encode, event->send_buf);
                    sgn_buf_append(event->http_send_buf, event->send_buf->buf, event->send_buf->data_len);
                }
                break;
            case SKEGN_FEED_TYPE:
                DBG("http post feed");
                if(strcmp(event->param->compress, "opus") == 0){
                    sgn_buf_reset(event->send_buf);
                    sgn_opus_encode_append(event->opus_encode, msg->data, msg->data_len, 0, event->send_buf);
                    sgn_buf_append(event->http_send_buf, event->send_buf->buf, event->send_buf->data_len);
                }else{
                    sgn_buf_append(event->http_send_buf, msg->data, msg->data_len);
                }
                break;
            case SKEGN_STOP_TYPE:
                DBG("http post stop");
                if(strcmp(event->param->compress, "opus") == 0){
                    sgn_buf_reset(event->send_buf);
                    sgn_opus_encode_append(event->opus_encode, msg->data, msg->data_len, 1, event->send_buf);
                    sgn_buf_append(event->http_send_buf, event->send_buf->buf, event->send_buf->data_len);
                }
                sprintf(url, "http:%s/%s", event->cfg->server+strlen("ws:"), event->param->core_type);
				LOG(LOG_ERROR, "HTTP addr :%s",url);
                DBG("HTTP addr :%s", url);
                sprintf(content_type, "Content-Type: multipart/form-data; boundary=%s\r\nRequest-Index: 0\r\n", BOUNDARY);
                sgn_buf_append_str(event->http_send_buf, "\r\n--");
                sgn_buf_append_str(event->http_send_buf, BOUNDARY);
                sgn_buf_append_str(event->http_send_buf, "--");
                struct mg_connect_opts opts;
                memset(&opts, 0, sizeof(opts));
                opts.user_data = event;
                reset_status(event);
//                event->http_connect_status = NET_BREAK;
                event->connect_time_stamp = time(NULL);
                nk_log(event, 1, 9, NULL);
                mg_connect_http_data_opt(&event->mgr, http_post_handler, opts, url, content_type, event->http_send_buf->buf, event->http_send_buf->data_len);
                event->stop_time_stamp = time(NULL);
                break;
        }
        free(msg);
    }
}

static void add_http_post_log_event(event_t *event)
{
    msg_t *msg = NULL;
    char url[4096];
    for(msg = sgn_queue_pop(&event->http_post_log_queue); msg!=NULL; msg = sgn_queue_pop(&event->http_post_log_queue)){
        memset(url, 0, sizeof(url));
        if(strlen(event->log_ip_addr))
        {
            char ip_addr[32] = "";
#ifdef USE_SSL
            sprintf(ip_addr,"%s%s%s","https://",event->log_ip_addr,"/bus");
#else
			sprintf(ip_addr,"%s%s%s","http://",event->log_ip_addr,"/bus");
#endif
            strncpy(url, ip_addr, strlen(ip_addr));
        }else{
#ifdef USE_SSL
            strncpy(url, SECURE_LOGBUS_SERVER, strlen(SECURE_LOGBUS_SERVER));
#else
			strncpy(url, DEFAULT_LOGBUS_SERVER, strlen(DEFAULT_LOGBUS_SERVER));
#endif
        }
        sscanf(msg->data,"%[^ ]", url+strlen(url));

        struct mg_connect_opts opts;
        memset(&opts, 0, sizeof(opts));
        opts.user_data = event;
        mg_connect_http_opt(&event->mgr, http_post_log_handler, opts,url, "Content-type: application/x-www-form-urlencoded\r\n", strchr(msg->data, ' ')+strlen(" "));  
        free(msg);
    }
}

static void add_pipe_event(event_t *event)
{
    struct mg_add_sock_opts pipe_opts;
    memset(&pipe_opts, 0, sizeof(pipe_opts));
    pipe_opts.user_data = event;
    if(1 != mg_socketpair(event->pipe, SOCK_STREAM))goto end;
    mg_add_sock_opt(&event->mgr, event->pipe[0], pipe_ev_handler, pipe_opts);
    event->pipe_is_ready = 1;
end:
    return;
}

static void *event_run_thread(void *thread_event) {
    event_t *event = (event_t *)thread_event;
    DBG("++++++++++++++++++++Start poll++++++++++++++++++:%d", event->is_running);
    for(; event->is_running; ){
        if(event->pipe_is_ready==0){
            add_pipe_event(event);
        }
        if(event->ws_connect_to_do > 0){
            DBG("connect websocket----------");
            add_ws_connect_event(event);
            event->ws_connect_status = NET_CONNECTING;
            event->ws_connect_to_do = 0;
        }
        add_http_connect_event(event);
        add_http_post_log_event(event);
        mg_mgr_poll(&event->mgr, 100);
    }
    mg_mgr_free(&event->mgr);
    DBG("++++++++++++++++++++Finish poll++++++++++++++++++");
    return NULL;
}


#ifdef USE_NATIVE

static void *event_run_thread_native(void *thread_event) {
    event_t *event = (event_t *)thread_event;
    check_download_provision(event);
    time_t timestamp = 0;
    DBG("++++++++++++++++++++Start native_catfish  poll++++++++++++++++++:%d", event->is_running);
    for(; event->is_running; ){

        if(sgn_check_provision(event->cfg->provision)==SGN_AUTH_USE_CATFISH && (time(NULL)-timestamp)>0){
            timestamp = time(NULL);    // 1s后才在下一次循环进入本逻辑
            if(strlen(sgn_provision_get_auth_addr(event->cfg->provision)) < 7){
                // auth_addr 长度小于7字节表明还未收到地址，需要通过接受广播来获取
                mg_mgr_init(&event->mgr_bind_catfish, NULL);
                udp_recv_broadcast(event);
                mg_mgr_poll(&event->mgr_bind_catfish, 200);
                mg_mgr_free(&event->mgr_bind_catfish);
            }
            if(sgn_provision_get_catfish_connected(event->cfg->provision)==0){
                //sgn_provision_get_catfish_connected返回0表示还未与catfish服务建立连接，需要建立连接
                tcp_auth_catfish(event);
            }
        }
        mg_mgr_poll(&event->mgr_native, 200);
    }
    mg_mgr_free(&event->mgr_native);
    DBG("++++++++++++++++++++Finish native_catfish poll++++++++++++++++++");
    return NULL;
}

#endif


void sgn_event_delete(event_t *event)
{
    if(event){
        if(event->thread_id != 0){
            DBG("++++++++++++++++++++pthread_join+++++++++++");
            pthread_join(event->thread_id, NULL);
        }
        if(event->thread_native_id != 0){
            DBG("++++++++++++++++++++pthread_join native+++++++++++");
            pthread_join(event->thread_native_id, NULL);
        }
        sgn_queue_delete(&event->ws_msg_queue);
        sgn_queue_delete(&event->http_post_queue);
        sgn_queue_delete(&event->http_post_log_queue);
        if(event->send_buf)sgn_buf_delete(event->send_buf);
        if(event->http_send_buf)sgn_buf_delete(event->http_send_buf);
        if(event->pipe_buf)sgn_buf_delete(event->pipe_buf);
        if(event->opus_encode)sgn_opus_encode_delete(event->opus_encode);
        if(event->param) sgn_param_delete(event->param);
#ifdef USE_NATIVE
        if(event->native)sgn_native_del(event->native);
#endif
        free(event);
    }
}

event_t *sgn_event_new(cfg_t* cfg)
{
    int ret = -1;
    event_t *event = (event_t *)calloc(1, sizeof(event_t));
    memset(event->log_ip_addr, 0, 32);
    DBG("");
    if(event == NULL)goto end;
    memset(event, 0, sizeof(*event));
    event->send_buf = sgn_buf_new();
    event->http_send_buf = sgn_buf_new();
    event->pipe_buf = sgn_buf_new();
    if(event->send_buf==NULL || event->pipe_buf==NULL || event->http_send_buf==NULL){
        goto end;
    }
    event->ws_msg_queue.next = NULL;
    event->is_running = 1;
    event->result_not_returned = 0;
    event->cfg = cfg;
    event->param = sgn_param_new();
    #ifdef __IPHONE_OS__
    sgn_is_wifi_work();            // 触发ios12.1.x 弹出app访问网络 提示框
    #endif
    ret = 0;
end:
    if(ret){
        sgn_event_delete(event);
        event = NULL;
    }
    return event;
}

int sgn_event_init(event_t *event) {
    int ret = -1;
    DBG("");
    strncpy(event->ws_cur_core_type, "sent.eval", SGN_MIN_MIN);
    mg_mgr_init(&event->mgr, NULL);

#ifdef USE_NATIVE
    if(event->cfg->native_enable){
        mg_mgr_init(&event->mgr_native, NULL);
        ret = pthread_create(&event->thread_native_id, NULL, event_run_thread_native, event);
        if(ret < 0)     goto end;
    }
#endif

    add_pipe_event(event);
    if(event->cfg->cloud_enable){
        add_check_download_sdk_cfg_event(event);
        add_http_connect_event(event);
    }
    add_http_post_log_event(event);
    ret = pthread_create(&event->thread_id, NULL, event_run_thread, event);
end:

    return ret;
}

