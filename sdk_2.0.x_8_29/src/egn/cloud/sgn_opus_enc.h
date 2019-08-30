/*
 * engine.h
 *
 *  Created on: 2018年8月21日
 *      Author: weicong.liu
 */
#ifndef SRC_CLOUD_SGN_OPUS_ENC_H_
#define SRC_CLOUD_SGN_OPUS_ENC_H_


struct sgn_opus_encode;
#include "lib/sgn_buf.h"

struct sgn_opus_encode *sgn_opus_encode_new(int samplerate);
int sgn_opus_encode_start(struct sgn_opus_encode *opus_enc, struct sgn_buf *out_buf);
int sgn_opus_encode_append(struct sgn_opus_encode *opus_enc, char *data_in, int in_data_len, int eof, struct sgn_buf *out_buf);
void sgn_opus_encode_delete(struct sgn_opus_encode *opus_enc);

#endif
