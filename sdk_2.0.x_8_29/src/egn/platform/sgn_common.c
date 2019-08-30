#include <string.h>
#include <stdlib.h>
#include "skegn.h"

#ifdef __WIN32__

#else
#include <sys/utsname.h>
int sgn_get_arch(char sgn_arch[64])
{
    char *p = NULL;
    struct utsname host;
    uname(&host);
    strcpy(sgn_arch, host.machine);
    if(NULL != (p=strchr(sgn_arch, '\n'))){*p = '\0';}
    if(NULL != (p=strchr(sgn_arch, '\r'))){*p = '\0';}
    return 0;
}

#endif

unsigned int sgn_get_full_version()
{
    static unsigned int i_ver = 0;
    if(i_ver) return i_ver;
    char *p = NULL;
    char *pointer = NULL;
    char c_ver[10] = {0};
    unsigned int var = 0;
    strcpy(c_ver, SKEGN_VERSION);
    p = strtok_r(c_ver, ".", &pointer);
    var = atoi(p);
    while(NULL != (p=strtok_r(NULL, ".", &pointer))){
        var <<= 8;
        var += atoi(p);
    }
    i_ver = var << 8;
    return i_ver;
}
