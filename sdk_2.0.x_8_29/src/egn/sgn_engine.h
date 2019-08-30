/*
 * engine.h
 *
 *  Created on: 2018年8月21日
 *      Author: weicong.liu
 */

#ifndef SRC_EGN_SGN_ENGINE_H_
#define SRC_EGN_SGN_ENGINE_H_

#include <pthread.h>

#include "third/mongoose/mongoose.h"
#include "lib/sgn_buf.h"
#include "skegn.h"
#include "lib/sgn_msg_queue.h"
#include "cloud/sgn_opus_enc.h"
#include "native/sgn_native.h"

#define SGN_MIN_LEN 128
#define SGN_MIN_MIN 32

#define DEFAULT_SERVER "ws://api.17kouyu.com:8080"
#define SECURE_SERVER "wss://api.17kouyu.com:8443/sent.eval?e=0&t=0"
#define SECURE_SDK_CFG "{\"serverList\":[\"wss://api.17kouyu.com:8443\",\"wss://gray.stkouyu.com:8442\"]}"
#define DEFAULT_SDK_CFG "{\"serverList\":[\"ws://106.15.206.165:8080\",\"ws://59.110.158.216:8080\",\"ws://114.215.100.106:8080\",\
                                          \"ws://116.62.215.6:8080\",\"ws://39.108.142.178:8080\",\
                                          \"ws://112.74.59.152:8080\",\"ws://47.93.187.154:8080\",\"ws://47.105.126.88:8080\"]}"
#define SECURE_SDK_CFG_ADDR "https://update.17kouyu.com/sdk.cfg"
#define DEFAULT_SDK_CFG_ADDR "http://update.17kouyu.com/sdk.cfg"
#define AUTH_DEVICE_ADDR "auth.17kouyu.com:8001/device"

#define SECURE_LOGBUS_SERVER "https://log.17kouyu.com/bus"
#define DEFAULT_LOGBUS_SERVER "http://log.17kouyu.com/bus"
//#define DEFAULT_LOGBUS_SERVER "http://192.168.0.12/bus"

#define BOUNDARY "BOUNDARY1234567890STKOUYUABCDEF"

#define SKEGN_DEFAUTL_TYPE  0
#define SKEGN_START_TYPE 	1
#define SKEGN_FEED_TYPE 	2
#define SKEGN_STOP_TYPE 	3
#define SKEGN_CANCEL_TYPE 	4
#define SKEGN_DELETE_TYPE 	5
#define SKEGN_HTTP_LOG_TYPE 6

#define WEBSOCKET_PROTOCOL  1
#define HTTP_PROTOCOL       2

#define WS_TO_CONNECT           1
#define WS_TO_CHANGE_CORE_TYPE  2
#define WS_TO_RECONNECT         3
#define WS_TO_RECONNECT_ALL     4
#define HTTP_TO_POST            5

#define NET_BREAK           0
#define NET_CONNECTING      1
#define NET_HANDKSHAKE      2
#define NET_CONNECTED       3

struct sgn_provision;

typedef struct {
    char app_key[SGN_MIN_LEN];
    char secret_key[SGN_MIN_LEN];
    char server[SGN_MIN_LEN];
    char sdk_cfg_url[SGN_MIN_LEN];
    int log_enable;
    int nk_log_enable;
    int cloud_enable;
    int native_enable;
    int vad_enable;
    int auto_switch_protocol;
    time_t connect_timeout;			// seconds default 20
    time_t server_timeout;			// seconds default 60
    time_t ping_space;				// seconds default 20
    struct sgn_buf *sdk_cfg;
#ifdef USE_NATIVE
    struct sgn_provision *provision;
    char auth_addr[SGN_MIN_LEN];
    char device_id[SGN_MIN_MIN*2];
    struct sgn_buf *provision_path;
    struct sgn_buf *res_path;
#endif
} cfg_t;

typedef struct callback_data
{
    void *user_data;
    skegn_callback callback;
    char token_id[64];
}callback_data_t;

typedef struct {
    int is_json:1;
    int has_request:1;
    int sound_intensity_enable:1;
    char server_type[SGN_MIN_MIN];
    char audio_type[SGN_MIN_MIN];
    char core_type[SGN_MIN_MIN];
    char compress[SGN_MIN_MIN];
    char token_id[SGN_MIN_MIN];
    char user_id[SGN_MIN_MIN*2];
#ifdef USE_NATIVE
    char serial_number[SGN_MIN_LEN];
#endif
    int sample_rate;
    char channel;
}param_t;


typedef struct{
    char log_ip_addr[32];
    cfg_t *cfg;
    param_t *param;
    callback_data_t callback[2];
//    struct mg_connection *ws_nc;
//    struct mg_connection *http_nc;
    struct mg_mgr mgr;
	struct mg_mgr mgr_native;
    struct mg_mgr mgr_bind_catfish;    
    pthread_t thread_id;
	pthread_t thread_native_id;
    int is_running;
    int server_type;            // 0:cloud 1:native
    int last_msg_type;
    struct sgn_opus_encode *opus_encode;
    sock_t pipe[2];             // pipe[1] to write , pipe[0] to read
    int pipe_is_ready;
    struct sgn_buf *pipe_buf;
    struct sgn_buf *send_buf;
    int result_not_returned;
    char protocol;

    time_t stop_time_stamp;
    time_t connect_time_stamp;
    time_t last_pong_time_stamp;

    int ws_server_count;
    int ws_20009;
    int ws_connect_status;
    int ws_connect_to_do;
    char ws_addr_str[SGN_MIN_LEN];
    char ws_cur_core_type[SGN_MIN_MIN];
    msg_t ws_msg_queue;
    char ws_encrypt_key[20];
    char ws_decrypt_key[20];
    char ws_enc;

//    int http_connect_status;
    msg_t http_post_queue;
    msg_t http_post_log_queue;

    struct sgn_buf *http_send_buf;
#ifdef USE_NATIVE
    struct sgn_native *native;
#endif
}event_t;

struct skegn {
    cfg_t *cfg;
    event_t *event;
};

cfg_t *sgn_cfg_new(const char *s_cfg);
void sgn_cfg_delete(cfg_t *cfg);
struct skegn *sgn_engine_new(cfg_t* cfg, event_t *event);
void sgn_engine_delete(struct skegn *engine);

#endif /* SRC_EGN_SGN_ENGINE_H_ */
