#ifndef SGN_NATIVE_EVAL_MODULE_H_
#define SGN_NATIVE_EVAL_MODULE_H_
#include "sgn_native.h"

#ifdef __cplusplus
extern "C" {
#endif

struct sgn_native_eval;

struct sgn_native_eval * sgn_native_eval_new(const char *cfg);
int sgn_native_eval_delete(struct sgn_native_eval *eval);
int sgn_native_eval_start(struct sgn_native_eval *eval, const char *param, sgn_native_callback_t *callback);
int sgn_native_eval_feed(struct sgn_native_eval *eval, const void *data, int size);
int sgn_native_eval_stop(struct sgn_native_eval *eval);
int sgn_native_eval_cancel(struct sgn_native_eval *eval);

#ifdef __cplusplus
}
#endif
#endif
