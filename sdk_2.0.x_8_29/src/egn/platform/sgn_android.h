#ifndef SGN_ANDROID_H_
#define SGN_ANDROID_H_

#include "jni/jni.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * get/set android device id
 * set internal static _device_id variable once, and get many times
 */
int sgn_get_device_id(char device_id[64], JNIEnv *jenv, jobject jcontext);
int sgn_get_app_path(char path[1024], JNIEnv *jenv, jobject jcontext);

#ifdef __cplusplus
}
#endif

#endif
