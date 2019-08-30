/*
 * sgn_msg_queue.h
 *
 *  Created on: 2018年8月23日
 *      Author: weicong.liu
 */

#ifndef SRC_EGN_LIB_SGN_MSG_QUEUE_H_
#define SRC_EGN_LIB_SGN_MSG_QUEUE_H_

//#include "engine.h"

typedef struct msg_s{
    int type;
    int data_len;
    void *user_data;
    struct msg_s *next;
    char data[];
}msg_t;

msg_t *sgn_new_msg(int type, const char *data, const int data_size, void *user_data);
void sgn_queue_push(msg_t *queue, msg_t *node);
msg_t *sgn_queue_pop(msg_t *queue);
void sgn_queue_delete(msg_t *queue);

#endif /* SRC_EGN_LIB_SGN_MSG_QUEUE_H_ */
