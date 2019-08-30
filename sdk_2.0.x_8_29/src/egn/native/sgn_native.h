#ifdef USE_NATIVE

#ifndef SGN_NATIVE_H_
#define SGN_NATIVE_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "include/skegn.h"

struct sgn_callback;
typedef void (*sgn_native_callback_f)(void *usr_data, int type, int size, void *data);

typedef struct sgn_native_callback
{
    void                  *user_data;
    sgn_native_callback_f callback;
}sgn_native_callback_t;


struct sgn_native;

struct sgn_native *sgn_native_new(void *engine, const char *cfg);
int sgn_native_del(struct sgn_native *native);
int sgn_native_start(struct sgn_native *native, const char *param, int vad);
int sgn_native_feed(struct sgn_native *native, const void *data, int size, int vad);
int sgn_native_stop(struct sgn_native *native, int vad);
int sgn_native_cancel(void *native);

#ifdef __cplusplus
}
#endif
#endif

#endif // USE_NATIVE
