#ifdef __ANDROID__

//#include "sgn_android.h"
#include "sgn_common.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

/*
 * get/set android device id
 * set internal static _device_id variable once, and get many times
 */
int
sgn_get_device_id(char device_id[64], JNIEnv *jenv, jobject jcontext)
{
    char *p;
    static char _device_id[64];

    const char *android_id = NULL, *imei = NULL, *serial = NULL;
    jstring     jandroid_id = NULL, jimei = NULL, jserial = NULL;
    jint        jversion = 0;

    if (!jenv || !jcontext || _device_id[0]) {
        goto end;
    }

    /* String androidId = android.provider.Settings.Secure.getString(getApplicationContext().getContentResolver(), android.provider.Settings.Secure.ANDROID_ID); */
    /* String imei = ((android.telephony.TelephonyManager) getApplicationContext().getSystemService(android.content.Context.TELEPHONY_SERVICE)).getDeviceId(); */
    /* String serial = android.os.Build.SERIAL; */

    jandroid_id = (*jenv)->CallStaticObjectMethod(jenv,
            (*jenv)->FindClass(jenv, "android/provider/Settings$Secure"),
            (*jenv)->GetStaticMethodID(jenv,(*jenv)->FindClass(jenv, "android/provider/Settings$Secure"), "getString", "(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;"),
            (*jenv)->CallObjectMethod(jenv, jcontext, (*jenv)->GetMethodID(jenv, (*jenv)->GetObjectClass(jenv, jcontext), "getContentResolver", "()Landroid/content/ContentResolver;")),
            (*jenv)->NewStringUTF(jenv, "android_id"));

    /* fuck, if android.permission.READ_PHONE_STATE is not set, java layer will throw an exception */
    jimei = (*jenv)->CallObjectMethod(jenv,
            (*jenv)->CallObjectMethod(jenv, jcontext, (*jenv)->GetMethodID(jenv, (*jenv)->GetObjectClass(jenv, jcontext), "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;"), (*jenv)->NewStringUTF(jenv, "phone")),
            (*jenv)->GetMethodID(jenv, (*jenv)->FindClass(jenv, "android/telephony/TelephonyManager"), "getDeviceId", "()Ljava/lang/String;"));
    if ((*jenv)->ExceptionOccurred(jenv)) {
        (*jenv)->ExceptionDescribe(jenv);
        (*jenv)->ExceptionClear(jenv);
        jimei = NULL;
    }

    jversion = (*jenv)->GetStaticIntField(jenv,
            (*jenv)->FindClass(jenv, "android/os/Build$VERSION"),
            (*jenv)->GetStaticFieldID(jenv, (*jenv)->FindClass(jenv, "android/os/Build$VERSION"), "SDK_INT", "I"));
    if (jversion >= 9) {
        jserial = (*jenv)->GetStaticObjectField(jenv,
                (*jenv)->FindClass(jenv, "android/os/Build"),
                (*jenv)->GetStaticFieldID(jenv, (*jenv)->FindClass(jenv, "android/os/Build"), "SERIAL", "Ljava/lang/String;"));
    }

    android_id = jandroid_id ? (*jenv)->GetStringUTFChars(jenv, jandroid_id, NULL) : NULL;
    imei       = jimei ? (*jenv)->GetStringUTFChars(jenv, jimei, NULL) : NULL;
    serial     = jserial ? (*jenv)->GetStringUTFChars(jenv, jserial, NULL) : NULL;

    if (android_id && strcmp(android_id, "") && strcmp(android_id, "9774d56d682e549c")) {
        strcpy(_device_id, android_id);
    } else if (imei && strcmp(imei, "")) {
        strcpy(_device_id, imei);
    } else if  (serial && strcmp(serial, "")) {
        strcpy(_device_id, serial);
    } else {
        strcpy(_device_id, "");
    }

    if (strlen(_device_id) < 8) {
        strcpy(_device_id, "");
    }

    if (android_id) {
        (*jenv)->ReleaseStringUTFChars(jenv, jandroid_id, android_id);
    }
    if (imei) {
        (*jenv)->ReleaseStringUTFChars(jenv, jimei, imei);
    }
    if (serial) {
        (*jenv)->ReleaseStringUTFChars(jenv, jserial, serial);
    }

    for (p = _device_id; *p; ++p) {
        *p = (char)tolower(*p);
    }

end:
    if (device_id) {
        strcpy(device_id, _device_id);
    }

    return 0;
}


int sgn_get_system_info(sgn_system_info_t *info)
{
    FILE *file = NULL;
    int i = 0;
    char buf[64] = {0};
    info->version = sgn_get_full_version();
    info->source = SOURCE;
    info->protocol = PROTOCOL;
    sgn_get_arch(info->arch);
    strcpy(info->os, PLATFORM);

    file = fopen("/system/build.prop", "r");
    if(NULL == file)return -1;
    while(fgets(buf, 64, file))
    {
        char *p = NULL;
        p = strchr(buf, '\n');
        if (p) {
            *p = '\0';
        }

        p = strchr(buf, '\r');
        if (p) {
            *p = '\0';
        }
        if(strncmp("ro.build.version.release=", buf, strlen("ro.build.version.release="))==0)
        {
            strcpy(info->os_version, buf+strlen("ro.build.version.release="));
            i++;
        }
        else if(strncmp("ro.product.model=", buf, strlen("ro.product.model="))==0)
        {
            strcpy(info->product, buf+strlen("ro.product.model="));
            i++;
        }
        if(2==i)break;
    }
    return 0;
}

int sgn_get_app_path(char path[1024], JNIEnv *jenv, jobject jcontext)
{
    static char _path[1024] = {0};
    const char *c_path = NULL;
    jstring jandroid_path = NULL;
    if(strlen(_path)>0 ||  !jenv || !jcontext){
        goto end;
    }
    jobject _obj = (*jenv)->CallObjectMethod(jenv, jcontext, (*jenv)->GetMethodID(jenv, (*jenv)->GetObjectClass(jenv, jcontext), "getExternalFilesDir", "(Ljava/lang/String;)Ljava/io/File;"), NULL);
    if(_obj != NULL){
        jandroid_path = (*jenv)->CallObjectMethod(jenv, _obj, (*jenv)->GetMethodID(jenv, (*jenv)->GetObjectClass(jenv, _obj), "getAbsolutePath", "()Ljava/lang/String;"));
        c_path = jandroid_path ? (*jenv)->GetStringUTFChars(jenv, jandroid_path, NULL) : NULL;
    }
    if(c_path){
        strcpy(_path, c_path);
    }
    if (jandroid_path) {
        (*jenv)->ReleaseStringUTFChars(jenv, jandroid_path, c_path);
    }
    end:
    if(path && strlen(_path)>0){
        sprintf(path, "%s/", _path);
    }
    return 0;
}

#endif
