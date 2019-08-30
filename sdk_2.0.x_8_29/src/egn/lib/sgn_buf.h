
#ifndef _YY_BUF_H
#define _YY_BUF_H

struct sgn_buf
{
    long long buf_len;
    long long data_len;
    char *buf;
};

void sgn_buf_delete(struct sgn_buf *buf);

struct sgn_buf *sgn_buf_new();

int sgn_buf_append(struct sgn_buf *d, const char *data, int datalen);

int sgn_buf_append_str(struct sgn_buf *d, const char *data);

void sgn_buf_reset(struct sgn_buf *d);

void sgn_buf_remove(struct sgn_buf *d, int start, int end);

#endif
