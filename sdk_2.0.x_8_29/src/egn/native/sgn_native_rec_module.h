#ifndef SGN_NATIVE_REC_MODULE_H_
#define SGN_NATIVE_REC_MODULE_H_
#include "sgn_native.h"

#ifdef __cplusplus
extern "C" {
#endif

struct sgn_native_rec;

struct sgn_native_rec * sgn_native_rec_new(const char *cfg);
int sgn_native_rec_delete(struct sgn_native_rec *eval);
int sgn_native_rec_start(struct sgn_native_rec *eval, const char *param, sgn_native_callback_t *callback);
int sgn_native_rec_feed(struct sgn_native_rec *eval, const void *data, int size);
int sgn_native_rec_stop(struct sgn_native_rec *eval);
int sgn_native_rec_cancel(struct sgn_native_rec *eval);

#ifdef __cplusplus
}
#endif
#endif
