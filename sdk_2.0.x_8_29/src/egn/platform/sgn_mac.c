#if (defined __APPLE__) && !(defined __IPHONE_OS__)

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sgn_common.h"

int sgn_get_device_id(char device_id[64])
{
    // "IOPlatformSerialNumber" = "C02HR0MADHJN"
    static char *cmd = "ioreg -l | awk '/IOPlatformSerialNumber/ { split($0, line, \"\\\"\"); printf(\"%s\\n\", line[4]); }'";
    FILE *file = NULL;
    char *p = NULL;
    static char buf[64] = { 0 };

    if (buf[0]) {
        goto end;
    }

	file = popen(cmd, "r");
	if (file == NULL) { goto end; }

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
    info->version = sgn_get_full_version();
    info->source = SOURCE;
    info->protocol = PROTOCOL;
    sgn_get_arch(info->arch);
    strcpy(info->os, PLATFORM);
    file = popen("sw_vers", "r");
    if(NULL == file)return -1;
    while(fgets(buf, sizeof(buf), file))
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
        if(0 == strncmp(buf, "ProductName:", strlen("ProductName:")))
        {
            strcpy(info->os, buf+strlen("ProductName:")+1);
        }
        else if(0 == strncmp(buf, "ProductVersion:", strlen("ProductVersion:")))
        {
            strcpy(info->os_version, buf+strlen("ProductVersion:")+1);
        }
    }
    strcpy(info->product, "");

    return 0;
}

#endif
