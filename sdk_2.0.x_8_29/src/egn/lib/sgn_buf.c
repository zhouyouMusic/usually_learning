#include "sgn_buf.h"

#include <string.h>
#include <stdlib.h>

#define DEFAULT_LEN 1024

void sgn_buf_delete(struct sgn_buf *buf)
{
    if(buf){
        if(buf->buf){
            free(buf->buf);
            buf->buf = NULL;
        }
        free(buf);
    }
}

struct sgn_buf *sgn_buf_new()
{
    struct sgn_buf *buf = (struct sgn_buf *)malloc(sizeof(*buf));
    if(buf == NULL)return buf;
    buf->buf =  (char *)malloc(DEFAULT_LEN);
    if(buf->buf == NULL){
        sgn_buf_delete(buf);
        return NULL;
    }
    buf->buf_len = DEFAULT_LEN;
    buf->data_len = 0;
    return buf;
}

int sgn_buf_append(struct sgn_buf *d, const char *data, int datalen)
{
    char *p;
    if(d->data_len+datalen > d->buf_len){
        d->buf_len += (10240 > datalen ? 10240 : datalen);
        p = realloc(d->buf, d->buf_len+1);
        if(p==NULL){
            d->buf_len -= (10240 > datalen ? 10240 : datalen);
            return -1;
        }
        d->buf = p;
    }
    memcpy(d->buf+d->data_len, data, datalen);
    d->data_len += datalen;
    return 0;
}

int sgn_buf_append_str(struct sgn_buf *d, const char *data)
{
    if(sgn_buf_append(d, data, strlen(data)) != 0)return -1;
    *(d->buf+d->data_len) = '\0';
    return 0;
}

void sgn_buf_remove(struct sgn_buf *d, int start, int end) {
    if (end > d->data_len)
        end = d->data_len;
    memmove(d->buf + start, d->buf + end, d->data_len - end);
    d->data_len -= (end - start);
}

void sgn_buf_reset(struct sgn_buf *d)
{
    if(d){
        d->data_len = 0;
    }
}
