#include "skegn.h"
#include "jni/jni.h"

#include <stddef.h>
#include <string.h>

#ifndef WIN32
#   undef  JNIEXPORT
#   define JNIEXPORT __attribute ((visibility("default")))
#endif

#ifdef __ANDROID__
#   include "src/egn/platform/sgn_android.h"
#endif

static JavaVM *JVM;


static int
_callback(const void *usrdata, const char *id, int type, const void *message, int size)
{
    int rv = -1;
    JNIEnv     *jenv;
    jobject    jobj;
    jclass     jcls;
    jmethodID  jmid;
    jbyteArray jid;
    jbyteArray jmessage;

    (*JVM)->AttachCurrentThread(JVM, (void **)&jenv, NULL);
    jobj = (jobject)usrdata;
    jcls = (*jenv)->GetObjectClass(jenv, jobj);
    jmid = (*jenv)->GetMethodID(jenv, jcls, "run", "([BI[BI)I");
    if (!jmid) {

        goto end;
    }

    jid  = (*jenv)->NewByteArray(jenv, strlen(id));
    jmessage = (*jenv)->NewByteArray(jenv, (jint)size);

    (*jenv)->SetByteArrayRegion(jenv, jid,  0, strlen(id), (jbyte *)id);
    (*jenv)->SetByteArrayRegion(jenv, jmessage, 0, (jint)size, (jbyte *)message);

    rv = (*jenv)->CallIntMethod(jenv, jobj, jmid, jid, (jint)type, jmessage, (jint)size);

end:
    (*JVM)->DetachCurrentThread(JVM);

    return rv;
}


static jint JNICALL
_get_device_id(JNIEnv *jenv, jclass ths, jbyteArray jdeviceid, jobject jcontext)
{
    int rv = 0;
    char divice_id[64] = {0};

#ifdef __ANDROID__
    if (!jcontext) {
        return -1;
    }
    rv = sgn_get_device_id(divice_id, jenv, jcontext);
#else
    rv = skegn_get_device_id(divice_id);
#endif

    if (!rv) {
        (*jenv)->SetByteArrayRegion(jenv, jdeviceid, 0, (jint)strlen(divice_id), (jbyte *)divice_id);
    }

    return (jint)rv;
}


static jlong JNICALL
_new(JNIEnv *jenv, jclass ths, jstring jcfg, jobject jcontext)
{
    char *cfg;
    struct skegn *engine;

#if !(defined DIABLE_PROVISION) && defined __ANDROID__
    if (jcontext) {
        sgn_get_device_id(NULL, jenv, jcontext);
        sgn_get_app_path(NULL, jenv, jcontext);
    } else {
        return 0;
    }
#endif
    cfg  = (char *)(*jenv)->GetStringUTFChars(jenv, jcfg, NULL);

    engine = skegn_new(cfg);
    (*jenv)->ReleaseStringUTFChars(jenv, jcfg, cfg);
    return (jlong)engine;
}


static jint JNICALL
_delete(JNIEnv *jenv, jclass ths, jlong eid)
{
    struct skegn *engine = (struct skegn *)eid;

    /*TODO:这里没有删除ref，有内存泄漏*/
    //(*jenv)->DeleteLocalRef(jenv, (jobject)(engine->callback.user_data));
    return (jint)skegn_delete(engine);
}


static jint JNICALL
_start(JNIEnv *jenv, jclass ths, jlong jegn, jstring jparam, jbyteArray jid, jobject jcallback, jobject jcontext)
{
    int rv = -1;
    char id[64] = {0};
    char *param = NULL;
    jobject jobj = NULL;
#if !(defined DISABLE_PROVISION) && defined __ANDROID__  && defined USE_NAVITE
    if(jegn && ((struct skegn *)jegn)->up){
        if(jcontext) {

            uploader_set_wifi_status(((struct skegn *)jegn)->up, sgn_is_wifi_work(jenv, jcontext));
        } else {
            return (jint)rv;
        }
    }
#endif
    jobj = (void *)(*jenv)->NewGlobalRef(jenv, jcallback);
    param = (char *)(*jenv)->GetStringUTFChars(jenv, jparam, NULL);
    rv = skegn_start((struct skegn *)jegn, param, id, jobj ? _callback : NULL, jobj);
    (*jenv)->ReleaseStringUTFChars(jenv, jparam, param);

    if (!rv) {
        (*jenv)->SetByteArrayRegion(jenv, jid, 0, (jint)strlen(id), (jbyte *)id);
    }
    return (jint)rv;
}


static jint JNICALL
_feed(JNIEnv *jenv, jclass ths, jlong jegn, jbyteArray jdata, jint jsize)
{
    int  rv   = -1;
    void *data = NULL;

    data = (void *)((*jenv)->GetByteArrayElements(jenv, jdata, NULL));
    if (data) {
        rv  = skegn_feed((struct skegn *)jegn, data, (int)jsize);
        (*jenv)->ReleaseByteArrayElements(jenv, jdata, (jbyte *)data, JNI_ABORT);
    }

    return (jint)rv;
}


static jint JNICALL
_stop(JNIEnv *jenv, jclass ths, jlong jegn)
{
    return (jint)skegn_stop((struct skegn *)jegn);
}


//static jint JNICALL
//_log(JNIEnv *jenv, jclass ths, jlong jegn, jstring jlog)
//{
//    int ret = 0;
//    char *log;
//
//    log = (char *)(*jenv)->GetStringUTFChars(jenv, jlog, NULL);
//    ret = skegn_log((struct skegn *)jegn, log);
//    (*jenv)->ReleaseStringUTFChars(jenv, jlog, log);
//
//    return (jint)ret;
//}

/* TODO: process SET OPT */
static jint JNICALL
_opt(JNIEnv *jenv, jclass ths, jlong jegn, jint jopt, jbyteArray jdata, jint jsize)
{
    int rv = 0;
    int size = 0;
    char buf[4096] = { 0 }, *buf2 = NULL, *p = buf;

/*     size = jsize > 4096 ? 4096 : jsize;
    (*jenv)->GetByteArrayRegion(jenv, jdata, 0, size, buf);
    rv = skegn_opt((struct skegn *)jegn, (int)jopt, buf, sizeof(buf));
    if (rv == sizeof(buf) && jsize > sizeof(buf)) {
        buf2 = calloc(1, jsize);
        (*jenv)->GetByteArrayRegion(jenv, jdata, 0, jsize, buf2);
        rv = skegn_opt((struct skegn *)jegn, (int)jopt, buf2, jsize);
        p = buf2;
    }

    if (rv > 0) {
        (*jenv)->SetByteArrayRegion(jenv, jdata, 0, (jint)rv > jsize ? jsize : (jint)rv, (jbyte *)p);
    }

    if (buf2) {
        free(buf2);
    } */

    return (jint)rv;
}

static jint JNICALL
_cancel(JNIEnv *jenv, jclass ths, jlong jegn)
{
    return (jint)skegn_cancel((struct skegn *)jegn);
}


static JNINativeMethod NATIVES[] = {
    {"skegn_get_device_id", "([BLjava/lang/Object;)I", _get_device_id},
    {"skegn_new","(Ljava/lang/String;Ljava/lang/Object;)J", _new},
    {"skegn_delete","(J)I", _delete},
    {"skegn_start","(JLjava/lang/String;[BLcom/stkouyu/SkEgn$skegn_callback;Ljava/lang/Object;)I", _start},
    {"skegn_feed","(J[BI)I", _feed},
//    {"skegn_log","(JLjava/lang/String;)I", _log},
    {"skegn_cancel","(J)I", _cancel},
    {"skegn_opt", "(JI[BI)I", _opt},
    {"skegn_stop","(J)I", _stop}
};


JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *jvm, void *reserved)
{
    int     rv = -1;
    JNIEnv *jenv;
    jclass  jcls;

    JVM = jvm;

    if ((*jvm)->GetEnv(jvm, (void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
        goto end;
    }

    jcls = (*jenv)->FindClass(jenv, "com/stkouyu/SkEgn");
    if (!jcls) {
        goto end;
    }

    if ((*jenv)->RegisterNatives(jenv, jcls, NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0])) != 0) {
        goto end;
    }

    rv = JNI_VERSION_1_6;

end:
    return (jint)rv;
}
