/*
 * sgn_event_loop.c
 *
 *  Created on: 2018年8月21日
 *      Author: weicong.liu
 */

#include <pthread.h>
#include "skegn.h"
#include "lib/sgn_buf.h"
#include "lib/sgn_msg_queue.h"
#include "lib/sgn_dbg.h"
#include "lib/sgn_sha1.h"
#include "lib/sgn_sound_intensity.h"
#include "lib/sgn_hmac.h"
#include "sgn_engine.h"
#include "native/sgn_auth.h"
#include "platform/sgn_common.h"

#include "third/cJSON/cJSON.h"
#include "third/mongoose/mongoose.h"

// 授权provision 下载处理
#ifdef USE_NATIVE

void native_handle_msg(msg_t *msg, event_t *event, int vad)
{
    switch(msg->type){
            case SKEGN_START_TYPE:
//                memcpy(&event->native_cb, msg->user_data, sizeof(event->native_cb));
//                free(msg->user_data);
                sgn_native_start(event->native, msg->data, vad);
                break;
            case SKEGN_FEED_TYPE:
                sgn_native_feed(event->native, msg->data, msg->data_len, vad);
                break;
            case SKEGN_STOP_TYPE:
                sgn_native_stop(event->native, vad);
                break;
    }
}

static void http_provision_ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
    struct http_message *hm = (struct http_message *) ev_data;
    cJSON *json=NULL, *item=NULL;
    char *data = NULL, *binary=NULL;
    int binary_len = 0;
    event_t *event = nc->user_data;
    FILE *file = NULL;
    switch (ev) {
        case MG_EV_CONNECT:
          if (*(int *) ev_data != 0) {
            LOG(LOG_ERROR,"connect() failed: %s", strerror(*(int *) ev_data));
          }
          break;
        case MG_EV_HTTP_REPLY:
          nc->flags |= MG_F_CLOSE_IMMEDIATELY;
          DBG("device get data:%.*s", hm->body.len, hm->body.p);
            DBG("provision path:%s", event->cfg->provision_path->buf);
            if((data = malloc(hm->body.len+1)) != NULL){
                strncpy(data, hm->body.p, hm->body.len+1);
                json = cJSON_Parse(data);
                if(json != NULL){
                    if((item = cJSON_GetObjectItem(json, "provision"))!=NULL && item->type==cJSON_String){
                        if((binary = malloc(strlen(item->valuestring))) != NULL){
                            cs_base64_decode(item->valuestring, strlen(item->valuestring), binary, &binary_len);
                            if(binary_len>0 && event->cfg->provision_path && (file = fopen(event->cfg->provision_path->buf, "wb"))!=NULL){
                                fwrite(binary, 1, binary_len, file);
                                fclose(file);
                                DBG("write provision:%s", event->cfg->provision_path->buf);
                                if(event->cfg->provision)sgn_provision_delete(event->cfg->provision);
                                event->cfg->provision = sgn_provision_new(event->cfg->provision_path->buf);
                            }
                            free(binary);
                        }
                    }
                    if((item = cJSON_GetObjectItem(json, "error"))!=NULL && item->type==cJSON_String){
                        if(strcmp(item->valuestring, "expired")==0 && sgn_check_provision(event->cfg->provision) == SGN_AUTH_NEED_ACTIVATE){
                            sgn_auth_set_expire(event->cfg->provision, event->cfg->provision_path->buf);
                        }
                    }
                    cJSON_Delete(json);
                }
                free(data);
            }

          break;
        case MG_EV_CLOSE:
            DBG("Server closed connection");
          break;
        default:
          break;
    }
}

static void udp_broadcast_handler(struct mg_connection *nc, int ev, void *ev_data) {
    event_t *event = nc->user_data;
    int i = 0;
    switch (ev) {
        case MG_EV_RECV:
            if((int) nc->recv_mbuf.len >64 )
            {
                for (i = 0; i < (int) nc->recv_mbuf.len; i++) nc->recv_mbuf.buf[i] ^= 0x80;
                sgn_provision_set_auth_addr(event->cfg->provision, (const char *)(nc->recv_mbuf.buf+64));
                nc->recv_mbuf.len = 0;
            }
            break;
        default:
            break;
    }
}

static void http_catfish_ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
    struct http_message *hm = (struct http_message *) ev_data;
    cJSON *json=NULL, *item=NULL;
    char *data = NULL, *binary=NULL;
    char buf[1024] = {0};
    char sig[64] = {0}; char got_sig[64] = {0};
    char timestamp[64] = {0};
    int binary_len = 0;
    event_t *event = nc->user_data;
    FILE *file = NULL;
    switch (ev) {
        case MG_EV_CONNECT:
            if (*(int *) ev_data != 0) {
                LOG(LOG_ERROR,"connect() failed: %s", strerror(*(int *) ev_data));
            }
            break;
        case MG_EV_HTTP_REPLY:
            DBG("device get data:%.*s", hm->body.len, hm->body.p);
            strncpy(buf, hm->body.p, hm->body.len>1024?1024:hm->body.len);
            json = cJSON_Parse(buf);
            if(json){
                if((item=cJSON_GetObjectItem(json, "timestamp"))!=NULL && item->type==cJSON_String){
                    strncpy(timestamp, item->valuestring, sizeof(timestamp)-1);
                }
                if((item=cJSON_GetObjectItem(json, "sig"))!=NULL && item->type==cJSON_String){
                    strncpy(got_sig, item->valuestring, sizeof(sig)-1);
                }
                cJSON_free(json);
                DBG("time:%s;got_sig:%s", timestamp, got_sig);
            }
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "yy: %s\n%s\n%s\n%s", event->cfg->app_key, timestamp, event->cfg->secret_key, event->cfg->device_id);
            sgn_sha1(buf, strlen(buf), sig);
            DBG("sdk_sig:%s", sig);
            if(strcmp(sig, got_sig)==0){
                sgn_provision_set_catfish_auth(event->cfg->provision, time(NULL));
            }else{
                sgn_provision_set_catfish_auth(event->cfg->provision, 0);
            }
            DBG("sgn_provision:%d", sgn_check_provision(event->cfg->provision));
            break;
        case MG_EV_CLOSE:
            DBG("Server closed connection");
            break;
        default:
            break;
    }
}

void check_download_provision(event_t *event)
{
    char buf[1024] = {0};
    char sig[64] = {0};
    char timestamp[64] = {0};
    struct mg_connect_opts opts;

    if(event->cfg->native_enable && sgn_check_provision(event->cfg->provision)<SGN_AUTH_EXPIRE_OK){
        sprintf(timestamp, "%ld", (unsigned long)time(NULL));
        sprintf(buf, "yy: %s\n%s\n%s\n%s", event->cfg->app_key, timestamp, event->cfg->secret_key, event->cfg->device_id);
        sgn_sha1(buf, strlen(buf), sig);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "appKey=%s&timestamp=%s&Id=%s&sig=%s", event->cfg->app_key, timestamp, event->cfg->device_id, sig);

        memset(&opts, 0, sizeof(opts));
        opts.user_data = event;
        mg_connect_http_opt(&event->mgr_native, http_provision_ev_handler, opts, AUTH_DEVICE_ADDR, NULL, buf);
    }
}

#define CATFISH_UDP_ADDR "udp://0.0.0.0:8809"
void udp_recv_broadcast(event_t *event)
{
    struct mg_bind_opts opts;
    memset(&opts, 0, sizeof(opts));
    opts.user_data = event;
    mg_bind_opt(&event->mgr_bind_catfish, CATFISH_UDP_ADDR, udp_broadcast_handler, opts);
}

struct sgn_auth_msg {
    unsigned short length;
    unsigned char type;   /* 2 : request, 3 : response */
    unsigned char result; /* error code */
    struct {
        char appkey[64];
        int timestamp;
        char device_id[64];
        char sig[64];
        char auth_id[64];
        char tm[64];
    } body;
};

static void tcp_handler(struct mg_connection *nc, int ev, void *ev_data) {
    event_t *event = nc->user_data;
    int i = 0;
    struct sgn_auth_msg pkg = { 0 };
    char sig[64] = {0};
    char timestamp[64] = {0};
    char buf[1024] = {0};
    static time_t last_time = 0;
    switch (ev) {
        case MG_EV_RECV:
            memcpy(&pkg, nc->recv_mbuf.buf, sizeof(struct sgn_auth_msg)>nc->recv_mbuf.len ? nc->recv_mbuf.len : sizeof(struct sgn_auth_msg));
            sprintf(buf, "%s\n%s\n%s\n", event->cfg->app_key, pkg.body.tm, event->cfg->secret_key);
            sgn_hmac(event->cfg->secret_key, strlen(event->cfg->secret_key), buf, strlen(buf), sig);
            if(strncmp(sig, pkg.body.sig, 64) == 0)
            {
                sgn_provision_set_catfish_auth(event->cfg->provision, time(NULL));
            }
            break;
        case MG_EV_POLL:
            if(time(NULL) - last_time >= 1){
                sprintf(timestamp, "%ld", (unsigned long)time(NULL));
                sprintf(buf, "%s\n%s\n%s\n", event->cfg->app_key, timestamp, event->cfg->secret_key);
                sgn_hmac(event->cfg->secret_key, strlen(event->cfg->secret_key), buf, strlen(buf), sig);
                pkg.length = sizeof(struct sgn_auth_msg);
                pkg.type = 2;
                pkg.result = 0;
                strncpy(pkg.body.appkey, event->cfg->app_key, 64);
                strncpy(pkg.body.device_id, event->cfg->device_id, 64);
                strncpy(pkg.body.sig, sig, 64);
                strncpy(pkg.body.tm, timestamp, 64);
                mg_send(nc, (const char *)&pkg, sizeof(struct sgn_auth_msg));
                last_time = time(NULL);
                sgn_provision_set_catfish_connected(event->cfg->provision, 1);
            }
            break;
        case MG_EV_CONNECT:
            sgn_provision_set_catfish_connected(event->cfg->provision, 1);
            break;
        case MG_EV_CLOSE:
            sgn_provision_set_catfish_connected(event->cfg->provision, 0);
            break;
        default:
            break;
    }
}

void tcp_auth_catfish(event_t *event)
{
    char buf[1024] = {0};
    sprintf(buf, "tcp://%s", sgn_provision_get_auth_addr(event->cfg->provision));
    struct mg_connect_opts opts;
    memset(&opts, 0, sizeof(opts));
    opts.user_data = event;
    DBG("tcp addr: %s", buf);
    mg_connect_opt(&event->mgr_native, buf, tcp_handler, opts);
}

static void http_auth_catfish2(event_t *event)
{
    char buf[1024] = {0};
    char sig[64] = {0};
    char timestamp[64] = {0};
    struct mg_connect_opts opts;
    sprintf(timestamp, "%ld", (unsigned long)time(NULL));
    sprintf(buf, "yy: %s\n%s\n%s\n%s", event->cfg->app_key, timestamp, event->cfg->secret_key, event->cfg->device_id);
    sgn_sha1(buf, strlen(buf), sig);
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "{\"appKey\":\"%s\", \"timestamp\":\"%s\", \"Id\":\"%s\", \"sig\":\"%s\"}", event->cfg->app_key, timestamp, event->cfg->device_id, sig);

    memset(&opts, 0, sizeof(opts));
    opts.user_data = event;
    mg_connect_http_opt(&event->mgr, http_catfish_ev_handler, opts, sgn_provision_get_auth_addr(event->cfg->provision), NULL, buf);
}

#endif
