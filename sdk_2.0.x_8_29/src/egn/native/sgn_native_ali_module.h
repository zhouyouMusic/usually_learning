#ifndef SGN_NATIVE_ALI_MODULE_H_
#define SGN_NATIVE_ALI_MODULE_H_
#include "sgn_native.h"

#ifdef __cplusplus
extern "C" {
#endif

struct sgn_native_ali;

struct sgn_native_ali * sgn_native_ali_new(const char *cfg);
int sgn_native_ali_delete(struct sgn_native_ali *eval);
int sgn_native_ali_start(struct sgn_native_ali *eval, const char *param, sgn_native_callback_t *callback);
int sgn_native_ali_feed(struct sgn_native_ali *eval, const void *data, int size);
int sgn_native_ali_stop(struct sgn_native_ali *eval);
int sgn_native_ali_cancel(struct sgn_native_ali *eval);

#ifdef __cplusplus
}
#endif
#endif
