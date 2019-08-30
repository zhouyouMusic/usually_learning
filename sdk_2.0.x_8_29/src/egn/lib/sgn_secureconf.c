#if !defined DISABLE_PROVISION || defined __WIN32__

#include "sgn_secureconf.h"
#include "sgn_sha1.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int
sgn_secureconf_decrypt2(const char *cipher, char **plain, int *size, const char *salt)
{
    int i;
    char *buf, sign1[40], sign2[40];

    if (*size <= 40) {
        return -1;
    }

    /* decrypt */
    buf = (char *)calloc(1, *size + 40 + strlen(salt));
    memcpy(buf, cipher, *size);
    for (i = 0; i < *size; i++) {
        buf[i] = ~buf[i];
    }

    /* check sign */
    memcpy(sign1, buf, 40);
    memmove(buf, buf + 40, *size - 40);
    memcpy(buf + *size - 40, salt, strlen(salt));
    sgn_sha1(buf, *size - 40 + strlen(salt), sign2);
    if (strncmp(sign1, sign2, 40) == 0) {
        *plain = buf;
        *size = *size - 40;
        *(*plain + *size) = '\0';
        return 0;
    } else {
        *plain = NULL;
        *size  = 0;
        free(buf);
        fprintf(stderr, "it's not security config\n");
        return -1;
    }
}


int
sgn_secureconf_encrypt2(const char *plain, char **cipher, int *size, const char *salt)
{
    int i;
    char *buf, sign[40];
    buf = (char *)calloc(1, *size + 40 + strlen(salt));

    /* sign: sha1(plain + salt) */
    memcpy(buf, plain, *size);
    memcpy(buf + *size, salt, strlen(salt));
    sgn_sha1(buf, *size + strlen(salt), sign);

    /* encrypt: sign + plain text */
    memcpy(buf, sign, 40);
    memcpy(buf + 40, plain, *size);
    for (i = 0; i < *size + 40; i++) {
        buf[i] = ~buf[i];
    }

    *cipher = buf;
    *size = *size + 40;
    return 0;
}


int
sgn_secureconf_decrypt(const char *cipher, char **plain, int *size)
{
    return sgn_secureconf_decrypt2(cipher, plain, size, "fuck you crack");
}


int
sgn_secureconf_encrypt(const char *plain, char **cipher, int *size)
{
    return sgn_secureconf_encrypt2(plain, cipher, size, "fuck you crack");
}

#endif
