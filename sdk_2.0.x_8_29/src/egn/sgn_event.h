/*
 * sgn_event_loop.h
 *
 *  Created on: 2018年8月23日
 *      Author: weicong.liu
 */

#ifndef SRC_EGN_SGN_EVENT_H_
#define SRC_EGN_SGN_EVENT_H_

#include "sgn_engine.h"
event_t *sgn_event_new(cfg_t* cfg);
void sgn_event_delete(event_t *event);
int sgn_event_init(event_t *event);

#endif /* SRC_EGN_SGN_EVENT_H_ */
