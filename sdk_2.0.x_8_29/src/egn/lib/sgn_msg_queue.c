/*
 * sgn_msg_queue.c
 *
 *  Created on: 2018年8月23日
 *      Author: weicong.liu
 */

#include <stdio.h>

#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "sgn_msg_queue.h"


msg_t *sgn_new_msg(int type, const char *data, const int data_size, void *cb)
{
    msg_t *msg = (msg_t *)calloc(1, sizeof(*msg)+data_size);
    if(msg){
        msg->type = type;
        msg->data_len = data_size;
        memcpy(msg->data, data, data_size);
        msg->user_data = cb;
    }
    return msg;
}

void sgn_queue_push(msg_t *queue, msg_t *node)
{
    if(node == NULL || queue == NULL)return;
//    printf("push type is %d\n", node->type);fflush(stdout);
    node->next = queue->next;
    queue->next = node;
}

msg_t *sgn_queue_pop(msg_t *queue)
{
    msg_t *tmp = NULL, *node = NULL, *pre_node=NULL;
    for(tmp=queue ;tmp->next!=NULL&&tmp->next->next!=NULL; tmp=tmp->next){}
    node = tmp->next;
    if(tmp != NULL){
        tmp->next = NULL;
    }
    return node;
}

void sgn_queue_delete(msg_t *queue)
{
    msg_t *node, *n_node;
    for(node=queue->next; node!=NULL; node=n_node){
        n_node = node->next;
        free(node);
    }
    queue->next = NULL;
}
