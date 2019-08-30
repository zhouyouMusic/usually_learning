#ifndef _RECORD_RESULT_H_
#define _RECORD_RESULT_H_


void downDataPlay(struct decode_frame *framInfo,struct buffer *buffer,struct mad_decoder*decoder,int*initFlag);
void record_recognition(void (*func_result)(char*,char**),char**resOut);

#endif
