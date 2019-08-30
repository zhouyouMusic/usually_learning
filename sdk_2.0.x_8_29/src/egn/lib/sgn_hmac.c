#ifdef USE_NATIVE

#include <string.h>
#include <stdlib.h>
#include "sgn_sha1.h"
#include "cJSON/cJSON.h"
#include "sgn_hmac.h"

#define HMAC_PAD_LEN 64    /* 对于sha1使用64个字节 */
#define HMAC_PAD_BUF 128
#define HMAC_SHA1_LEN 20

void sgn_hmac(char *key, int k_len, char *txt, int t_len, char *out)
{
    unsigned char k_ipad[HMAC_PAD_BUF] = { 0 }; /* 对于sha1只有64个字节有效 */
    unsigned char k_opad[HMAC_PAD_BUF] = { 0 };
    unsigned char *content = NULL;
    unsigned char digits[HMAC_SHA1_LEN] = { 0 };
    struct sha1_context sha;
    int i;
    int len;

    content = (unsigned char *)malloc(t_len + HMAC_PAD_LEN + 1);
    if (NULL == content)
    {
        return;
    }

    /* 如果key的长度大于了64，需要先做一次sha1 */
    if (k_len > HMAC_PAD_LEN)
    {
        sgn_sha1_init(&sha);
        sgn_sha1_update(&sha, (unsigned char *) key, k_len);
        sgn_sha1_final(&sha, digits);
        key = (char *)digits;
        k_len = HMAC_SHA1_LEN;
    }

    memcpy(k_ipad, key, k_len);
    memcpy(k_opad, key, k_len);

    /* ipad is the byte 0x36 repeated 64 times
       opad is the byte 0x5c repeated 64 times*/
    for (i = 0; i < HMAC_PAD_LEN; ++i)
    {
        k_ipad[i] ^= 0x36;
        k_opad[i] ^= 0x5c;
    }

    memset(content, 0, t_len + HMAC_PAD_LEN);
    memcpy(content, k_ipad, HMAC_PAD_LEN);
    memcpy(content + HMAC_PAD_LEN, txt, t_len);
    len = HMAC_PAD_LEN + t_len;

    /* fisrt sha1*/
    memset(out, 0, HMAC_SHA1_LEN);
    sgn_sha1_init(&sha);
    sgn_sha1_update(&sha, (unsigned char *)content, len);
    sgn_sha1_final(&sha, digits);

    memset(content, 0, t_len + HMAC_PAD_LEN);
    memcpy(content, k_opad, HMAC_PAD_LEN);
    memcpy(content + HMAC_PAD_LEN, digits, HMAC_SHA1_LEN);

    /* second sha1 */
    memset(out, 0, HMAC_SHA1_LEN);
    sgn_sha1(content, HMAC_SHA1_LEN + HMAC_PAD_LEN, out);

    free(content);
}

#endif
