#ifndef SGN_NATIVE_VAD_MODULE_H_
#define SGN_NATIVE_VAD_MODULE_H_
#include "sgn_native.h"

#ifdef __cplusplus
extern "C" {
#endif

struct sgn_native_vad;

struct sgn_native_vad * sgn_native_vad_new(const char *cfg);
int sgn_native_vad_delete(struct sgn_native_vad *vad);
int sgn_native_vad_start(struct sgn_native_vad *vad, const char *param, sgn_native_callback_t *callback);
int sgn_native_vad_feed(struct sgn_native_vad *vad, const void *data, int size);
int sgn_native_vad_stop(struct sgn_native_vad *vad);
int sgn_native_vad_cancel(struct sgn_native_vad *vad);

#ifdef __cplusplus
}
#endif
#endif
