#ifndef SGN_NATIVE_OPEN_MODULE_H_
#define SGN_NATIVE_OPEN_MODULE_H_
#include "sgn_native.h"

#ifdef __cplusplus
extern "C" {
#endif

struct sgn_native_open;

struct sgn_native_open * sgn_native_open_new(const char *cfg);
int sgn_native_open_delete(struct sgn_native_open *open);
int sgn_native_open_start(struct sgn_native_open *open, const char *param, sgn_native_callback_t *callback);
int sgn_native_open_feed(struct sgn_native_open *open, const void *data, int size);
int sgn_native_open_stop(struct sgn_native_open *open);
int sgn_native_open_cancel(struct sgn_native_open *open);

#ifdef __cplusplus
}
#endif
#endif
