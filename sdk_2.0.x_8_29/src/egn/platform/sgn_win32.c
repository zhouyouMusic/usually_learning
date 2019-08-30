
#ifdef __WIN32__

//#include "sgn_win32.h"
#include "lib/sgn_secureconf.h"
#include "../platform/sgn_common.h"

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <windows.h>
#include <shlobj.h>
/* #include <wininet.h> */
#include <direct.h>
#define snprintf _snprintf

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>


#define UDIDINFO_FORMAT(udidinfo) "udid:%s\nmac:%s\nbiosid:%s\nboardid:%s\ncpuid:%s\ndiskid:%s\n", (udidinfo)->udid, (udidinfo)->mac, (udidinfo)->biosid, (udidinfo)->boardid, (udidinfo)->cpuid, (udidinfo)->diskid


struct udidinfo {
    char udid[256];
    char mac[256];
    char biosid[256];
    char boardid[256];
    char cpuid[256];
    char diskid[256];
};


static char *
_strnorm(char *str) {
    char *p, *p2;
    for (p = str; *p;) {
        if (isalnum(*p)) {
            *p = tolower(*p);
            p++;
        } else {
            for (p2 = p; p2 && *p2; p2++) {
                *p2 = *(p2 + 1);
            }
        }
    }
    return str;
}


static unsigned int
_strhash(const char *p)
{
    unsigned int hash = 0;
    while (*p) {
        hash = hash * 131 + *p++;
    }

    return hash;
}


static int
_tmpnam(char s[MAX_PATH])
{
    int rv = -1;
    char tmpath[MAX_PATH];
    rv = (int)GetTempPath(MAX_PATH, tmpath);
    if (rv > MAX_PATH || rv == 0) {
        rv = -1;
        goto end;
    }

    rv = (int)GetTempFileName(tmpath, "sgn", 0, s);
    if (rv == 0) {
        rv = -1;
        goto end;
    }

    rv = 0;
end:
    return rv;
}


/*
 * http://msdn.microsof.com/en-us/library/windows/desktp/ms682499%28v=vs.85%29.aspx
 * http://www.codeprojeateProcess-and-wait-for-reslt
 * http://support.microoft.com/kb/190351
 * http://stackoverflow.com/questions/10210553/cmd-is-somehow-writing-chinese-text-as-output
 */
static int
_system(const char *cmd, char *output, int size, int timeout)
{
    int rv = 0;
    char tmp[MAX_PATH] = {0}, cmd2[4096] = {0};
    FILE *file = NULL;

    DWORD exitcode = 0;

    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};

    output[0] = '\0';
    rv = _tmpnam(tmp);
    if (rv) {
        fprintf(stderr, "failed to get tmpnam\n");
        goto end;
    }

    snprintf(cmd2, sizeof(cmd2), "cmd /C \"%s 2>NUL | more >%s\"", cmd, tmp);

    rv = (int)CreateProcessA(NULL, (char *)cmd2, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    if (!rv) {
        fprintf(stderr, "failed to create process\n");
        rv = -1;
        goto end;
    }

    rv = (int)WaitForSingleObject(pi.hProcess, timeout);
    if (rv == WAIT_FAILED) {
        fprintf(stderr, "wait process failed\n");
        rv = -1;
        goto end;
    } else if (rv == WAIT_TIMEOUT) {
        TerminateProcess(pi.hProcess, 0);
        fprintf(stderr, "wait process timeout\n");
        rv = -1;
        goto end;
    }

    rv = GetExitCodeProcess(pi.hProcess, &exitcode);
    if (!rv) {
        fprintf(stderr, "failed to get exit code\n");
        rv = -1;
        goto end;
    }

    if (exitcode != 0) {
        fprintf(stderr, "invalid exit code: %d\n", (int)exitcode);
        rv = -1;
        goto end;
    }

    file = fopen(tmp, "rb");
    if (!file) {
        fprintf(stderr, "failed to open: %s\n", tmp);
        rv = -1;
        goto end;
    }

    rv = (int)fread(output, 1, size, file);
    if (rv == 0) {
        fprintf(stderr, "failed to read: %s\n", tmp);
        rv = -1;
        goto end;
    }

    output[rv] = '\0';

    rv = 0;

end:
    if (pi.hProcess) {
        CloseHandle(pi.hProcess);
    }

    if (pi.hThread) {
        CloseHandle(pi.hThread);
    }

    if (file) {
        fclose(file);
    }

    if (tmp[0]) {
        remove(tmp);
    }

    return rv;
}


static int
_wmic(const char *cmd, char *output, int size)
{
    int rv = -1;
    char buf[4096] = {0};

    output[0] = '\0';
    rv = _system(cmd, buf, sizeof(buf) - 1, 10000);
    if (!rv) {
        int is_blank_line, is_first_line, is_first_value;
        char *p, *line_start, *line_end, *value_start, *value_end, *buf_end;

        is_first_line = 1;
        is_first_value = 1;

        is_first_value = 1;
        for (line_start = buf, buf_end = buf + strlen(buf) - 1;
                line_start <= buf_end;
                line_start = line_end + 2) {

            is_blank_line = 1;
            line_end = strchr(line_start, '\n');
            line_end = line_end ? (line_end - 1) : buf_end;

            if (is_first_line) {
                is_first_line = 0;
                continue;
            }

            for (p = line_start; p <= line_end; p++) {
                if (!isspace(*p)) {
                    if (is_blank_line) {
                        is_blank_line = 0;
                        value_start = p;
                    }
                    value_end = p;
                }
            }

            if (!is_blank_line) {
                if ((size - strlen(output)) < (value_end - value_start + 2)) {
                    break;
                }

                if (is_first_value) {
                    is_first_value = 0;
                } else {
                    strcat(output, ",");
                }

                p = output + strlen(output);
                strncat(p, value_start, value_end - value_start + 1);
                _strnorm(p);
            }
        }
    }

    return rv;
}


static int
udidinfo_write_to_file(struct udidinfo *udidinfo, const char *path)
{
    int rv = -1, size;
    char *p, buf[4096];
    FILE *file;

    snprintf(buf, sizeof(buf) - 1, UDIDINFO_FORMAT(udidinfo));

    size = (int)strlen(buf);
    rv = sgn_secureconf_encrypt2(buf, &p, &size, "rb"); /* rb is salt */
    if (!rv) {
        file = fopen(path, "wb");
        if (file) {
            fwrite(p, 1, size, file);
            fclose(file);
            rv = 0;
        }
        free(p);
    }

    return rv;
}


static int
udidinfo_get_from_file(struct udidinfo *udidinfo, const char *path)
{
    int rv = -1, size;
    char *p, buf[4096];
    char *pointer = NULL;
    FILE *file;
    file = fopen(path, "rb");
    if (file) {
        size = (int)fread(buf, 1, sizeof(buf) - 1, file);
        rv = sgn_secureconf_decrypt2(buf, &p, &size, "rb"); /* rb is salt */
        if (!rv) {
            strncpy(buf, p, size);
            for (p = buf; ; p = NULL) {
                p = strtok_r(p, "\n", &pointer);
                if (!p) {
                    break;
                }

                if (p == strstr(p, "udid:")) {
                    sscanf(p, "udid:%s", udidinfo->udid);
                } else if (p == strstr(p, "mac:")) {
                    sscanf(p, "mac:%s", udidinfo->mac);
                } else if (p == strstr(p, "biosid:")) {
                    sscanf(p, "biosid:%s", udidinfo->biosid);
                } else if (p == strstr(p, "boardid:")) {
                    sscanf(p, "boardid:%s", udidinfo->boardid);
                } else if (p == strstr(p, "cpuid:")) {
                    sscanf(p, "cpuid:%s", udidinfo->cpuid);
                } else if (p == strstr(p, "diskid:")) {
                    sscanf(p, "diskid:%s", udidinfo->diskid);
                }
            }

            free(p);
            rv = 0;
        }
        fclose(file);
    }

    return rv;
}


extern int sgn_win32_getmac(char mac[256]);

static int
udidinfo_get_from_hardware(struct udidinfo *udidinfo)
{
    int i;
    char *p, buf[4096];
    char *pointer = NULL;

    sgn_win32_getmac(udidinfo->mac);

    _wmic("wmic bios get serialnumber", udidinfo->biosid, 255);
    _wmic("wmic baseboard get serialnumber", udidinfo->boardid, 255);
    _wmic("wmic cpu get processorid", udidinfo->cpuid, 255);
    _wmic("wmic diskdrive get serialnumber", udidinfo->diskid, 255);

    if (!udidinfo->udid[0]) {

        /* try to use mac as device id firstly, then cpuid */
        snprintf(buf, sizeof(buf), "%s", udidinfo->mac[0] ? udidinfo->mac : udidinfo->cpuid);

        for (p = buf; ; p = NULL) {
            p = strtok_r(p, ",", &pointer);
            if (!p) {
                break;
            }

            snprintf(udidinfo->udid, sizeof(udidinfo->udid) - 1, "%s", p);
            break;
        }
    }

    /* otherwise use hardware id hash to generate a id */
    if (!udidinfo->udid[0]) {
        sprintf(udidinfo->udid + 0, "%2.2x", _strhash(udidinfo->biosid) % 256);
        sprintf(udidinfo->udid + 2, "%2.2x", _strhash(udidinfo->boardid) % 256);
        sprintf(udidinfo->udid + 4, "%2.2x", _strhash(udidinfo->cpuid) % 256);
        sprintf(udidinfo->udid + 6, "%2.2x", _strhash(udidinfo->diskid) % 256);

        srand(time(NULL));
        for (i = 8; i < 12; i++) {
            sprintf(udidinfo->udid + i, "%1.1x", rand() % 16);
        }
    }

    return 0;
}


static int
_idcmp(const char *id1, const char *id2)
{
    char *p, buf[256];
    char *pointer = NULL;

    if (!id1[0] && !id2[1]) {
        return 0;
    }

    strncpy(buf, id1, sizeof(buf) - 1);
    for (p = buf; ; p = NULL) {
        p = strtok_r(p, ",", &pointer);
        if (!p) {
            break;
        }
        if (strstr(id2, p)) {
            return 0;
        }
    }

    strncpy(buf, id2, sizeof(buf) - 1);
    for (p = buf; ; p = NULL) {
        p = strtok_r(p, ",", &pointer);
        if (!p) {
            break;
        }
        if (strstr(id1, p)) {
            return 0;
        }
    }

    return 1;
}


static int
udidinfo_compare(struct udidinfo *udidinfo1, struct udidinfo *udidinfo2)
{
    int changes = 0;

    if (strlen(udidinfo1->udid) < 8 ||  strlen(udidinfo2->udid) < 8) { /* invalid udid */
        return -1;
    }

    if (!strcmp(udidinfo1->udid, udidinfo2->udid)) { /* udid equals */
        return 0;
    }

    changes += _idcmp(udidinfo1->mac, udidinfo2->mac);
    changes += _idcmp(udidinfo1->biosid, udidinfo2->biosid);
    changes += _idcmp(udidinfo1->boardid, udidinfo2->boardid);
    changes += _idcmp(udidinfo1->cpuid, udidinfo2->cpuid);
    changes += _idcmp(udidinfo1->diskid, udidinfo2->diskid);

    if (changes <= 3) {
        return 0;
    } else {
        return changes;
    }
}


int
sgn_get_device_id(char device_id[64])
{
    static char _device_id[64];

    int rv = -1;
    char path[512];

    struct udidinfo udidinfo;
    struct udidinfo udidinfo2;

    if (_device_id[0] != '\0') {
        rv = 0;
        goto end;
    }

    memset(&udidinfo, 0, sizeof(struct udidinfo));
    memset(&udidinfo2, 0, sizeof(struct udidinfo));

    rv = (int)SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, path);
    if (rv) {
        fprintf(stderr, "failed to get common appdata folder path\n");
        goto end;
    }

    strcat(path, "\\skegn");
    rv = access(path, F_OK);
    if (rv) {
        rv = mkdir(path);
        if (rv) {
            fprintf(stderr, "failed to create: %s\n", path);
            goto end;
        }
        SetFileAttributes(path, FILE_ATTRIBUTE_HIDDEN);
    }

    strcat(path, "\\udidinfo");
    rv = access(path, F_OK);
    if (rv) { /* generate udid once */
        udidinfo_get_from_hardware(&udidinfo2);
        rv = udidinfo_write_to_file(&udidinfo2, path);
        if (rv) {
            fprintf(stderr, "failed to write udidinfo\n");
            goto end;
        }

        SetFileAttributes(path, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY);
    }

    if (!udidinfo2.udid[0]) {
        udidinfo_get_from_hardware(&udidinfo2);
    }

    rv = udidinfo_get_from_file(&udidinfo, path);
    if (rv) {
        fprintf(stderr, "failed to read udidinfo");
        if(access(path, F_OK|W_OK|R_OK) == 0)
        {
            unlink(path);
            sprintf(_device_id, "%s", udidinfo2.udid);
            rv = 0;
        }
        goto end;
    }

    // if (!udidinfo2.udid[0]) {
    //     udidinfo_get_from_hardware(&udidinfo2);
    // }

#ifdef DEBUG_DEVICE_ID
    printf("\n---- udidinfo from file ----\n");
    printf(UDIDINFO_FORMAT(&udidinfo));

    printf("\n---- udidinfo from hardware ----\n");
    printf(UDIDINFO_FORMAT(&udidinfo2));
#endif

    rv = udidinfo_compare(&udidinfo, &udidinfo2);
    if (rv) {
        fprintf(stderr, "failed to compare udidinfo, computer may changed!\n");
        goto end;
    }

    sprintf(_device_id, "%s", udidinfo.udid);
    rv = 0;
end:
    device_id[0] = '\0';
    if (!rv) {
        strcpy(device_id, _device_id);
    }
    return rv;
}

int sgn_get_system_info(sgn_system_info_t *info)
{
    SYSTEM_INFO siSysInfo;
    OSVERSIONINFOEX osvi;
    BOOL bOsVersionInfoEx;
    info->version = sgn_get_full_version();
    info->source = SOURCE;
    info->protocol = PROTOCOL;
    GetSystemInfo(&siSysInfo);
    sprintf(info->arch, "%d", (unsigned int)siSysInfo.dwProcessorType);

    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*)&osvi);
    if(!bOsVersionInfoEx)
        return -1;
    sprintf(info->os_version,"%d.%d(%d)", (unsigned int)(osvi.dwMajorVersion),
             (unsigned int)(osvi.dwMinorVersion), (unsigned int)(osvi.dwBuildNumber));
    sprintf(info->os, "%s%d", info->os_version, (unsigned int)osvi.wProductType);
    strcpy(info->product, "");

    return 0;
}

int sgn_get_app_path(char path[1024])
{
    int ret;
    ret = (int)SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, path);
    if (ret) {
        return ret;
    }
    strcat(path, "\\skegn\\");
    return ret;
}
#endif
