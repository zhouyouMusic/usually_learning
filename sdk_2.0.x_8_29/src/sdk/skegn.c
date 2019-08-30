/*
 * skegn.c
 *
 *  Created on: 2018年8月21日
 *      Author: weicong.liu
 *
 * Description: API
 *
 */
#ifndef SKEGN_LOG_DEBUG
#define SKEGN_LOG_DEBUG LOG_WARN
#endif
#include "skegn.h"

#include "../egn/platform/sgn_common.h"
#include "lib/sgn_dbg.h"
#include "sgn_engine.h"
#include "sgn_event.h"
#include "lib/sgn_uuid.h"
#include "third/cJSON/cJSON.h"

SKEGN_IMPORT_OR_EXPORT struct skegn * SKEGN_CALL
skegn_new(const char *s_cfg)
{
	struct skegn *egn = NULL;
	msg_t *msg = NULL;
	sgn_log_set_level(SKEGN_LOG_DEBUG);
	DBG("%s", s_cfg);
	cfg_t *cfg = sgn_cfg_new(s_cfg);
	if(cfg == NULL)goto end;
	event_t *event = sgn_event_new(cfg);
	if(event == NULL)goto end;
	if(sgn_event_init(event))goto end;
	egn = sgn_engine_new(cfg, event);
#ifdef USE_NATIVE           // 发送消息通知事件处理模块加载离线内核
	if(egn==NULL)goto end;
	if(cfg->native_enable || cfg->vad_enable){
        msg = sgn_new_msg(SKEGN_DEFAUTL_TYPE, "", 0, NULL);
        if(msg==NULL)goto end;
        send(event->pipe[1], (const char *)&msg, sizeof(msg), 0);
    }
#endif
end:
    if(egn == NULL){
        LOG(LOG_ERROR, ("skegn_new failed:%s", s_cfg));
        sgn_cfg_delete(cfg);
        sgn_event_delete(event);
    }
    DBG("skegn_new end:%p", egn);
	return egn;
}


SKEGN_IMPORT_OR_EXPORT int SKEGN_CALL
skegn_delete(struct skegn *egn)
{
    int ret = -1;
    FILE *fd = NULL;
    DBG("skegn_delete");
    msg_t *msg = NULL;
    if(egn==NULL)goto end;
    msg = sgn_new_msg(SKEGN_DELETE_TYPE, "", 0, NULL);
    if(msg==NULL)goto end;
	ret = send(egn->event->pipe[1], (const char *)&msg, sizeof(msg), 0);
	if(ret <= 0 || ret != sizeof(msg))
	{
		goto end;
	}
    sgn_engine_delete(egn);
    ret = 0;
end:
    DBG("skegn_delete end:%d", ret);
    fd = sgn_log_get_file();
    if(fd)fclose(fd);
    if(ret)LOG(LOG_ERROR, ("skegn_delete failed"));
    return ret;
}


SKEGN_IMPORT_OR_EXPORT int SKEGN_CALL
skegn_start(struct skegn *egn,
                   const char *s_param,
                   char id[64],
                   skegn_callback callback,
                   const void *usrdata)
{
    int ret = -1;
	msg_t *msg = NULL;
	DBG("skegn_start");
	if(egn == NULL || s_param==NULL || id==NULL || callback==NULL)goto end;
	callback_data_t *cb = malloc(sizeof(*cb));
	if(cb == NULL)goto end;
	cb->callback = callback;
	cb->user_data = (void *)usrdata;
	memset(id, 0, 64);    uuidgen2(id);
	strncpy(cb->token_id, id, 64);

	msg = sgn_new_msg(SKEGN_START_TYPE, s_param, strlen(s_param)+1, cb);
	if(msg==NULL)goto end;
	ret = send(egn->event->pipe[1], (const char *)&msg, sizeof(msg), 0);
	if(ret <= 0 || ret != sizeof(msg))
    {
        goto end;
    }
	ret = 0;
end:
    DBG("skegn_start end:%d", ret);
    if(ret)LOG(LOG_ERROR, ("skegn_start failed"));
    return ret;
}


SKEGN_IMPORT_OR_EXPORT int SKEGN_CALL
skegn_feed(struct skegn *egn, const void *p_data, int size)
{
	int ret = -1;
	msg_t *msg = NULL;
	if(egn == NULL || p_data==NULL || size<0)goto end;
	msg = sgn_new_msg(SKEGN_FEED_TYPE, p_data, size, NULL);
	if(msg==NULL)goto end;
	ret = send(egn->event->pipe[1], (const char *)&msg, sizeof(msg), 0);
	if(ret <= 0 || ret != sizeof(msg))
	{
	    goto end;
	}
	ret = 0;
end:
    if(ret)LOG(LOG_ERROR, ("skegn_feed failed"));
    return ret;
}

SKEGN_IMPORT_OR_EXPORT int SKEGN_CALL
skegn_stop(struct skegn *egn)
{
    int ret = -1;
	msg_t *msg = NULL;
	DBG("skegn_stop");
	if(egn == NULL)goto end;
	msg = sgn_new_msg(SKEGN_STOP_TYPE, "", 0, NULL);
	if(msg==NULL)goto end;
	ret = send(egn->event->pipe[1], (const char *)&msg, sizeof(msg), 0);
	if(ret <= 0 || ret != sizeof(msg))
    {
        goto end;
    }
	ret = 0;
end:
    DBG("skegn_stop end:%d", ret);
    if(ret)LOG(LOG_ERROR, "skegn_stop failed");
    return ret;
}


SKEGN_IMPORT_OR_EXPORT int SKEGN_CALL
skegn_cancel(struct skegn *egn)
{
    int ret = -1;
    msg_t *msg = NULL;
    if(egn == NULL)goto end;
    msg = sgn_new_msg(SKEGN_CANCEL_TYPE, "", 0, NULL);
    if(msg==NULL)goto end;
	ret = send(egn->event->pipe[1], (const char *)&msg, sizeof(msg), 0);
	if(ret <= 0 || ret != sizeof(msg))
	{
		goto end;
	}
	ret = 0;
end:
    DBG("skegn_cancel end:%d", ret);
    return ret;
}

SKEGN_IMPORT_OR_EXPORT int SKEGN_CALL
skegn_get_device_id(char device_id[64])
{
    int ret = -1;
    char *tmp = device_id;
    if (NULL == device_id)
    {
        goto end;
    }
    memset(device_id, 0, 64);
#if defined __ANDROID__
    ret = sgn_get_device_id(device_id, NULL, NULL);
#elif defined __IPHONE_OS__ || __APPLE__ || __LINUX__ || __WIN32__
    ret = sgn_get_device_id(device_id);
#else
    strcpy(device_id, "unknown-platform-device");
    goto end;
#endif
    while ('\0' != *tmp)
    {
        *tmp = (char)tolower(*tmp);
        ++tmp;
    }
end:
    return ret;
}

SKEGN_IMPORT_OR_EXPORT int SKEGN_CALL
skegn_opt(struct skegn *engine, int opt, char *data, int size)
{
//    if(opt == SKEGN_GET_SERIAL_NUMBER)
//        return sgn_get_provision(data, size);
//    else
        return 0;
}
