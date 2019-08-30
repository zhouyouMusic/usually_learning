#ifndef COMMON_H_
#define COMMON_H_

#include "skegn.h"


#if defined __ANDROID__
    #define PLATFORM "android"
    #define SOURCE 1
    #include "../platform/sgn_android.h"
#elif defined __IPHONE_OS__
    #define PLATFORM "ios"
    #define SOURCE 5
    #include "../platform/sgn_iphoneos.h"
#elif defined __APPLE__
    #define PLATFORM "darwin"
    #define SOURCE 3
    #include "../platform/sgn_mac.h"
#elif defined __LINUX__
    #define SOURCE 6
    #define PLATFORM "linux"
    #include "../platform/sgn_linux.h"
#elif defined __WIN32__
    #define SOURCE 7
    #define PLATFORM "windows"
    #include "../platform/sgn_win32.h"
#else
    #error "unknown_platform"
#endif
#define PROTOCOL 1

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sgn_system_info
{
    char is_get;
    unsigned int version;   //sdk version
    char source;    //sdk target platform
    char protocol;
    char arch[20];
    char os[20];
    char os_version[64];
    char product[20];   //product name such as "ipad"
}sgn_system_info_t;

int sgn_get_arch(char sgn_arch[64]);
int sgn_get_system_info(sgn_system_info_t *info);
unsigned int sgn_get_full_version();

#ifdef __cplusplus
}
#endif
#endif
