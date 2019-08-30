/*
 * sgn_native_event.h
 *
 *  Created on: 2019年2月23日
 *      Author: weicong.liu
 */

#ifndef SRC_EGN_SGN_NATIVE_EVENT_H_
#define SRC_EGN_SGN_NATIVE_EVENT_H_

#include "sgn_engine.h"
void native_handle_msg(msg_t *msg, event_t *event, int vad);
void tcp_auth_catfish(event_t *event);
void udp_recv_broadcast(event_t *event);
void check_download_provision(event_t *event);

#endif /* SRC_EGN_SGN_NATIVE_EVENT_H_ */
