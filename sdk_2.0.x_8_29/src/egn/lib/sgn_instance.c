
#ifndef DISABLE_PROVISION

#include "sgn_instance.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef __WIN32__
#   include <windows.h>
#else
#   include <fcntl.h>
#   include <unistd.h>
#endif


struct sgn_instance {
    char name[64];
#ifdef __WIN32__
    HANDLE mutex;
#else
    int fd;
    struct flock flock;
#endif
};


#ifndef __WIN32__
static const char *
_tmpdir()
{
    static char tmpdir[256];
    char *p; 

    if (tmpdir[0]) {
        return tmpdir;
    }

    tmpnam(tmpdir);
    p = strrchr(tmpdir, '/');
    if (p) {
        *p = '\0';
    }   

    p = strrchr(tmpdir, '\\');
    if (p) {
        *p = '\0';
    }

    return tmpdir;
}
#endif


/* 检测实例数是否超出证书文件中允许的上限 */
struct sgn_instance *
sgn_instance_apply_for(char *app_key, int max)
{
    int i = 0, rv;
    struct sgn_instance *token = NULL;
    char name[64] = {0};

#ifdef __WIN32__
    HANDLE mutex = INVALID_HANDLE_VALUE;

    for (i = 0; i < max; ++i) {
        snprintf(name, sizeof(name), "skegn-%s-%d", app_key, i);
        mutex = CreateMutex(NULL, FALSE, name);
        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            if (mutex != INVALID_HANDLE_VALUE) {
                CloseHandle(mutex);
                mutex = INVALID_HANDLE_VALUE;
            }
            continue;
        } else {
            break;
        }
    }

    if (mutex != INVALID_HANDLE_VALUE) {
        token = (struct sgn_instance *)calloc(1, sizeof(*token));
        token->mutex = mutex;
        memcpy(token->name, name, sizeof(name));
    }
#else
    int fd = 0;
    struct flock flock = {0};
    for (i = 0; i < max; ++i) {
        snprintf(name, sizeof(name), "%s/skegn-%s-%d", _tmpdir(), app_key, i);
        fd = open(name, O_WRONLY | O_CREAT, 0200);
        if (fd <= 0) {
            continue;
        }

        flock.l_type = F_WRLCK;
        rv = fcntl(fd, F_SETLK, &flock);
        if (rv) {
            close(fd);
            fd = 0;
            continue;
        }

        break;
    }

    if (fd > 0) {
        token = (struct sgn_instance *)calloc(1, sizeof(*token));
        token->fd = fd;
        memcpy(&token->flock, &flock, sizeof(struct flock));
        memcpy(token->name, name, sizeof(name));
    }
#endif

    return token;
}

/* 删除文件锁  */
int
sgn_instance_give_back(struct sgn_instance *token)
{
#ifdef __WIN32__
    if (token->mutex != INVALID_HANDLE_VALUE) {
        CloseHandle(token->mutex);
        token->mutex = INVALID_HANDLE_VALUE;
    }
#else
    if (token->fd) {
        close(token->fd);
        if (token->flock.l_pid > 0) {
            token->flock.l_type = F_UNLCK;
            fcntl(token->fd, F_SETLK, &token->flock);
        }
    }
#endif

    free(token);
    return 0;
}

#endif
