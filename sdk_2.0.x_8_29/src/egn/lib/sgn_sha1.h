#ifndef SKEGN_SHA1_H_
#define SKEGN_SHA1_H_

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/* POINTER defines a generic pointer type */
typedef unsigned char * POINTER;

/* UINT4 defines a four byte word */
typedef uint32_t UINT4;

/* BYTE defines a unsigned character */
typedef unsigned char BYTE;

struct sha1_context
{
	UINT4 digest[5];            /* Message digest */
	UINT4 countLo, countHi;       /* 64-bit bit count */
	UINT4 data[16];             /* SHS data buffer */
	int Endianness;
};

void sgn_sha1_init(struct sha1_context *shsInfo);
void sgn_sha1_final(struct sha1_context *shsInfo, unsigned char *output);
void sgn_sha1_update(struct sha1_context *shsInfo, unsigned char *buffer, int count);
void sgn_sha1(const char *message, int size, char sig[40]);
void sgn_sha1_20(const char *message, int size, char hash_out[20]);

#ifdef __cplusplus
}
#endif
#endif