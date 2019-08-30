#ifdef __LINUX__

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sgn_common.h"

int sgn_get_device_id(char device_id[64])
{
    // /sys/class/dmi/id/product_uuid
    // 7402D403-3504-D405-8B06-5D0700080009
    FILE *file = NULL;
    char *p = NULL;
    static char buf[64] = { 0 };

    if (buf[0]) {
        goto end;
    }

    file = fopen("/sys/class/dmi/id/product_uuid", "rb");
    if (!file) {
        goto end;
    }

    fread(buf, 1, sizeof(buf) - 1, file);
    fclose(file); file = NULL;

    for (p = buf; *p;) {
        if (isalnum(*p)) {
            *p = (char)tolower(*p);
            ++p;
        } else if (*p == '\n') {
            *p = '\0';
        } else {
            char *q = NULL;
            for (q = p; *q; ++q) {
                *q = *(q + 1);
            }
        }
    }

end:
    if (device_id) {
        strcpy(device_id, buf);
    }

    return 0;
}


int sgn_get_system_info(sgn_system_info_t *info)
{
    FILE *file = NULL;
    char buf[64] = {0};
    char *p = NULL;
    info->version = sgn_get_full_version();
    info->source = SOURCE;
    info->protocol = PROTOCOL;
    sgn_get_arch(info->arch);
    strcpy(info->os, PLATFORM);
    file = popen("head -n 1 /etc/issue", "r");
    if(NULL == file)return -1;
    fgets(buf, sizeof(buf), file);
    if(NULL != (p=strchr(buf, '\n'))){*p = '\0';}
    if(NULL != (p=strchr(buf, '\r'))){*p = '\0';}
    p = strchr(buf, ':');
    if(NULL != p)
    {
        p += 2;
        strcpy(info->os_version, p);
    }
    strcpy(info->product, "");
    if(NULL != file)pclose(file);
    return 0;
}

#endif
