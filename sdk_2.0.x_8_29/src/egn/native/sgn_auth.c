#ifndef DISABLE_PROVISION

#ifdef USE_NATIVE
#include "sgn_auth.h"
#include "skegn.h"

#include "lib/sgn_sha1.h"
#include "cJSON/cJSON.h"
#include "lib/sgn_secureconf.h"
#include "lib/sgn_file.h"
#include "lib/sgn_instance.h"
#include "lib/sgn_dbg.h"
#include "platform/sgn_common.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>

#ifdef __ANDROID__
#   include <regex.h>
#   include "platform/sgn_android.h"
#endif
#ifdef __WIN32__
#include <windows.h>
#include <shlobj.h>
#endif

#define DEFAULT_INSTANCE_NUMBER 1

struct sgn_provision {
    char version[64];           /* optional, skegn version */
    char app_key[64];           /* required, appKey */
    char secret_key[64];        /* optional, secretKey, default "", only used in catfish service */
    char platform[64];          /* 可选，限制证书使用平台，默认不限制，可选android,ios,darwin,windows,linux */
    char devid_id[64];          /* optional, check device id, only current device can use this provision file */
    time_t expire;              /* optional, expire date, default 0, means no limit */
    time_t timestamp;
    int max_instance_number_per_device;     /* optional, default 0, means no limit */
    struct sgn_instance *instance;       /* optional, one instance one token */
    char (*native_cores)[64];   /* optional, provisioned native-invoke core types, default [] */
    int use_tuna;
    int use_catfish;
    int need_activate;
    int is_catfish_connected;
    time_t catfish_success_time;// 记录catfish auth自上次成功的时间戳
    char auth_address[64];      /* optional, default "" */
};

static time_t _str2time(const char *str)
{
    struct tm tm = {0};
    sscanf(str, "%d-%d-%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
    tm.tm_year -= 1900;
    tm.tm_mon  -= 1;
    return mktime(&tm);
}

static void _time2str(char str[64], time_t tm)
{
    if(str != NULL){
        struct tm *p = localtime(&tm);
        sprintf(str, "%d-%d-%d", 1900+p->tm_year, 1+p->tm_mon, p->tm_mday);
    }
}

static int
_load_provision(struct sgn_provision *provision, const char *path)
{
    int rv = -1, i, n, size;
    int to_ansi = 0;
    char *cipher = NULL, *plain = NULL;
    FILE *file = NULL;
    char pro_path[PATH_MAX] = { 0 };
    cJSON *json = NULL, *jsonarray = NULL, *jsonitem = NULL;

#ifdef __WIN32__
    if (0 == sgn_is_file_exist(path))
    {
        sgn_utf8_to_ansi(path, pro_path);
        to_ansi = 1;
    }
#endif
    if (0 == to_ansi)
    {
        memcpy(pro_path, path, strlen(path));
    }

    file = fopen(pro_path, "rb");
    if (!file) {
        goto end;
    }

    fseek(file, 0, SEEK_END);
    size = (int) ftell(file);
    cipher = (char *)malloc(size);
    fseek(file, 0, SEEK_SET);
    if (fread(cipher, 1, size, file) != size) {
        goto end;
    }

    rv = sgn_secureconf_decrypt(cipher, &plain, &size);
    if (rv) { 
        goto end;
    }

    json = cJSON_Parse(plain);
    if (!json) {
        rv = -1;
        goto end;
    }
    jsonitem = cJSON_GetObjectItem(json, "deviceId");
    memset(provision->devid_id, 0, sizeof(provision->devid_id));
    if (jsonitem && jsonitem->type == cJSON_String) {
        memcpy(provision->devid_id, jsonitem->valuestring, strlen(jsonitem->valuestring));
    }

    jsonitem = cJSON_GetObjectItem(json, "version");
    if (jsonitem && jsonitem->type == cJSON_String) {
        strncpy(provision->version, jsonitem->valuestring, sizeof(provision->version) - 1);
    }

    jsonitem = cJSON_GetObjectItem(json, "appKey");
    if (jsonitem && jsonitem->type == cJSON_String) {
        strncpy(provision->app_key, jsonitem->valuestring, sizeof(provision->app_key) - 1);
    }

    jsonitem = cJSON_GetObjectItem(json, "secretKey");
    if (jsonitem && jsonitem->type == cJSON_String) {
        strncpy(provision->secret_key, jsonitem->valuestring, sizeof(provision->secret_key) - 1);
    }

    jsonitem = cJSON_GetObjectItem(json, "expire");
    if (jsonitem && jsonitem->type == cJSON_String) {
        provision->expire = _str2time(jsonitem->valuestring);
    }

    jsonitem = cJSON_GetObjectItem(json, "timestamp");
    if (jsonitem && jsonitem->type == cJSON_String) {
        provision->timestamp = _str2time(jsonitem->valuestring);
    }

    jsonitem = cJSON_GetObjectItem(json, "platform");
    if (jsonitem && jsonitem->type == cJSON_String) {
        strncpy(provision->platform, jsonitem->valuestring, sizeof(provision->platform) - 1);
    }
    jsonarray = cJSON_GetObjectItem(json, "nativeInvokeCoreTypes");
    if (jsonarray  && jsonarray->type == cJSON_Array) {
        n = cJSON_GetArraySize(jsonarray);
        provision->native_cores = (char (*)[64])calloc(1, sizeof(*provision->native_cores) * (n + 1));
        for (i = 0; i < n; i++) {
            jsonitem = cJSON_GetArrayItem(jsonarray, i);
            strncpy(provision->native_cores[i], jsonitem->valuestring, sizeof(*provision->native_cores) - 1);
        }
    }

    jsonitem = cJSON_GetObjectItem(json, "maxInstanceNumberPerDevice");
    if (jsonitem && jsonitem->type == cJSON_Number && jsonitem->valueint > 0) {
        provision->max_instance_number_per_device = jsonitem->valueint;
    }
#if (defined __APPLE__ || __WIN32__ || __LINUX__) && !(defined __IPHONE_OS__)
    else if(jsonitem == NULL)
    {
        provision->max_instance_number_per_device = DEFAULT_INSTANCE_NUMBER;
    }
#endif

    jsonitem = cJSON_GetObjectItem(json, "useCatfish");
	if (jsonitem && jsonitem->type == cJSON_Number) {
		provision->use_catfish = jsonitem->valueint;
	}
	jsonitem = cJSON_GetObjectItem(json, "localAuthAddress");
	if (jsonitem && jsonitem->type == cJSON_String) {
		strncpy(provision->auth_address, jsonitem->valuestring, sizeof(provision->auth_address));
	}
    jsonitem = cJSON_GetObjectItem(json, "useTuna");
	if (jsonitem && jsonitem->type == cJSON_Number) {
		provision->use_tuna = jsonitem->valueint;
	}
    jsonitem = cJSON_GetObjectItem(json, "needActivate");
	if (jsonitem && jsonitem->type == cJSON_Number) {
		provision->need_activate = jsonitem->valueint;
	}

end:
    if (file)   fclose(file);
    if (cipher) free(cipher);
    if (plain)  free(plain);

    if (json) {
        cJSON_Delete(json);
    }

    return rv;
}

int sgn_write_provision_to_file(struct sgn_provision *provision, const char *path)
{
    int rv = -1, size;
    char *p, *str=NULL;
    FILE *file;
    cJSON *pro_json = NULL, *item = NULL;
    char buf[64];
    if(provision == NULL || path == NULL)return rv;
    if((pro_json = cJSON_CreateObject()) != NULL){
        if(strlen(provision->app_key) >  0){
            cJSON_AddStringToObject(pro_json, "appKey", provision->app_key);
        }
        if(provision->expire >=  0){
            memset(buf, 0, sizeof(buf));
            _time2str(buf, provision->expire);
            cJSON_AddStringToObject(pro_json, "expire", buf);
        }
        if(provision->timestamp >  0){
            memset(buf, 0, sizeof(buf));
            _time2str(buf, provision->timestamp);
            cJSON_AddStringToObject(pro_json, "timestamp", buf);
        }
        if(provision->use_catfish != 0){
            cJSON_AddNumberToObject(pro_json, "useCatfish", provision->use_catfish);
        }
        if(provision->use_tuna != 0){
            cJSON_AddNumberToObject(pro_json, "useTuna", provision->use_tuna);
        }
        if(provision->need_activate != 0){
            cJSON_AddNumberToObject(pro_json, "needActivate", provision->need_activate);
        }
        if(provision->max_instance_number_per_device != DEFAULT_INSTANCE_NUMBER){
            cJSON_AddNumberToObject(pro_json, "maxInstanceNumberPerDevice", provision->max_instance_number_per_device);
        }
        if(strlen(provision->devid_id) >  0){
            cJSON_AddStringToObject(pro_json, "deviceId", provision->devid_id);
        }
        if(strlen(provision->version) >  0){
            cJSON_AddStringToObject(pro_json, "version", provision->version);
        }
        if(strlen(provision->secret_key) >  0){
            cJSON_AddStringToObject(pro_json, "secretKey", provision->secret_key);
        }
        if(strlen(provision->platform) >  0){
            cJSON_AddStringToObject(pro_json, "platform", provision->platform);
        }
        if(strlen(provision->auth_address) >  0){
            cJSON_AddStringToObject(pro_json, "localAuthAddress", provision->auth_address);
        }
        if(provision->native_cores != NULL){
            int i;
            cJSON *array = cJSON_CreateArray();
            for(i=0; strlen(provision->native_cores[i])>0; i++){
            	cJSON_AddItemToArray(array, cJSON_CreateString(provision->native_cores[i]));
            }
            cJSON_AddItemToObject(pro_json, "nativeInvokeCoreTypes", array);
        }
        str = cJSON_PrintUnformatted(pro_json);
        DBG("write provision: %s", str);
        cJSON_Delete(pro_json);
    }
    if(str != NULL){
        size = (int)strlen(str);
        rv = sgn_secureconf_encrypt(str, &p, &size); /* rb is salt */
        if (!rv) {
            file = fopen(path, "wb");
            if (file) {
                fwrite(p, 1, size, file);
                fclose(file);
                rv = 0;
            }
            free(p);
        }
        free(str);
    }
    return rv;
}

struct sgn_provision *sgn_provision_new(const char *path)
{
    int rv;
    struct sgn_provision *provision;
    if(path==NULL || strlen(path)==0)return NULL;
    provision = (struct sgn_provision *)calloc(1, sizeof(*provision));

    rv =_load_provision(provision, path);
    if (rv) {
        sgn_provision_delete(provision);
        provision = NULL;
    } else {
        if (provision->max_instance_number_per_device && provision->app_key) {
            provision->instance = sgn_instance_apply_for(provision->app_key, provision->max_instance_number_per_device);
        }
    }
    return provision;
}


int sgn_provision_delete(struct sgn_provision *provision)
{
    if(provision){
        if (provision->native_cores) {
            free(provision->native_cores);
        }

        if (provision->instance) {
            sgn_instance_give_back(provision->instance);
        }
        free(provision);
    }
    return 0;
}


static int _generate_serial_number(char serial_number[64], const char *app_key, const char *secret_key, const char *device_id)
{
    char buf[256], sig[64];
    sprintf(buf, "yy\n: appKey: %s, secretKey: %s, deviceId: %s\r\n", app_key, secret_key, device_id);
    sgn_sha1(buf, strlen(buf), sig); /* sha1 with salt */
    sprintf(serial_number, "%.4s-%.4s-%.4s-%.4s-%.4s", sig, sig + 4, sig + 8, sig + 12, sig + 16);
    return 0;
}

static int is_local_auth_address_public(const char *auth_addr)
{
	int ret = -1;
	char addr[16] = {0};
	char a[4] = {0}, b[4] = {0};
	char *p1=NULL, *p2 = NULL;
	int i;
	strncpy(addr, auth_addr, 15);
	p1 = strchr(addr, '.');
	if(p1 != NULL){
		p2 = strchr(p1+1, '.');
		if(p2 == NULL)goto end;
	}else{
		goto end;
	}
	*p1 = '\0';
	*p2 = '\0';
	strncpy(a, addr, 3);
	strncpy(b, p1+1, 3);
	if(strcmp(a, "10")==0 || (strcmp(a, "192")==0&&strcmp(b,"168")==0) || (strcmp(a, "172")==0&&(atoi(b)>15&&atoi(b)<32))){
		ret = 0;
	}
end:
	return ret;
}

int sgn_auth_verify(struct sgn_provision *provision, const char *app_key, const char *secret_key, const char *core_type, char **error)
{
    int rv = -1;
    int i, has_permission = 0;
    char (*cores)[64] = NULL;
    FILE *file = NULL;
    if (!provision) {
        *error = "auth failed, invalid provision profile";
        goto end;
    }

    if (strcmp(provision->version, "") && strcmp(provision->version, SKEGN_VERSION))
    {
        *error = "auth failed, invalid provision profile, invalid version";
        goto end;
    }

    if (strcmp(provision->platform, "") && strcmp(provision->platform, PLATFORM)) {
        *error = "auth failed, invalid provision profile, invalid platform";
        goto end;
    }

//    if ((0 != strlen(provision->devid_id)) && (!device_id || strcmp(provision->devid_id, device_id)))
//    {
//        *error = "auth failed, not match the device id";
//        goto end;
//    }

    if (!strcmp(provision->app_key, "")) {
        *error = "auth failed, invalid provision profile, no appKey";
        goto end;
    }

    if (!app_key || !strcmp(app_key, "")) {
        *error = "auth failed, no appKey";
        goto end;
    }
    if (!secret_key || !strcmp(secret_key, "")){
        *error = "auth failed, no secretKey";
        goto end;
    }
    if (strcmp(provision->app_key, app_key)) {
        *error = "auth failed, invalid appKey";
        goto end;
    }

    if (provision->use_catfish){
        if(time(NULL) - provision->catfish_success_time > 3){
            *error = "auth failed, catfish auth failed";
            goto end;
        }
    }else if(provision->expire<=0 || provision->expire < time(NULL) || provision->timestamp > time(NULL)) {
        *error = "auth failed, license has expired or system time is error";
        goto end;
    }

    if (provision->max_instance_number_per_device && !provision->instance) {
        *error = "auth failed, reaches the limit of instance number";
        goto end;
    }

    cores = provision->native_cores;
    has_permission = (!core_type || !core_type[0]) ? 1 : 0;
    for (i = 0; core_type && cores && strcmp(cores[i], ""); ++i) {
        if (!strcmp(core_type, cores[i])) {
            has_permission = 1;
            break;
        }
    }
    if (!has_permission) {
        *error = "auth failed, no permission to access this coreType";
        rv = -1;
        goto end;
    }

    rv = 0;
end:
    if(NULL != file)fclose(file);
    return rv;
}

int sgn_check_provision(struct sgn_provision *provision)
{
    if(provision == NULL){
        return SGN_AUTH_NULL;
    }else if(provision->need_activate){
        return SGN_AUTH_NEED_ACTIVATE;
    }else if(provision->use_catfish){
        return SGN_AUTH_USE_CATFISH;
    }else if(provision->expire-time(NULL) < 18*60*60){      // 18 hours
        DBG("%d,provision->expire:%d", provision->expire-time(NULL), provision->expire);
        return SGN_AUTH_EXPIRE_LESS_18H;
    }else if(provision->expire-time(NULL) < 30*24*60*60){   // 30 days
        return SGN_AUTH_EXPIRE_LESS_30D;
    }else{
        return SGN_AUTH_EXPIRE_OK;
    }
}

const char *
sgn_provision_get_app_key(struct sgn_provision *provision)
{
    return provision->app_key;
}

const char *
sgn_provision_get_secret_key(struct sgn_provision *provision)
{
    return provision->secret_key;
}

const char *
sgn_provision_get_auth_addr(struct sgn_provision *provision)
{
    DBG("sgn_provision_get_auth_addr:%s", provision->auth_address);
    return provision->auth_address;
}

void sgn_provision_set_auth_addr(struct sgn_provision *provision, const char *addr)
{
    if(provision && addr){
        strncpy(provision->auth_address, addr, sizeof(provision->auth_address));
    }
}

void sgn_provision_set_catfish_auth(struct sgn_provision *provision, time_t value)
{
    DBG("sgn_provision_set_catfish_auth:%d", value);
    provision->catfish_success_time = value;
}

time_t sgn_provision_get_catfish_auth(struct sgn_provision *provision)
{
    return provision->catfish_success_time;
}

void sgn_provision_set_catfish_connected(struct sgn_provision *provision, int value)
{
    DBG("sgn_provision_set_catfish_connected:%d", value);
    provision->is_catfish_connected = value;
}

int sgn_provision_get_catfish_connected(struct sgn_provision *provision)
{
    return provision->is_catfish_connected;
}

void sgn_activate_provision(struct sgn_provision *provision, const char *path)
{
    provision->need_activate = 0;
    sgn_write_provision_to_file(provision, path);
}
void sgn_auth_set_expire(struct sgn_provision *provision, const char *path)
{
    provision->expire = 0;
    sgn_write_provision_to_file(provision, path);
}
#endif

#endif /* no DISABLE_PROVISION */
