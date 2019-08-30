#ifndef UUID_H_
#define UUID_H_

#include <time.h>

/* a simple implementation of uuid */
#ifdef __cplusplus
extern "C" {
#endif

/* generate 12 byte binary uuid */
void uuidgen(char uuid[12]);

/* generate 12 byte binary uuid, and convert it to 24 hex string */
void uuidgen2(char hexstr[24]);

void uuid_from_string(char hexstr[24], char uuid[12]);
void uuid_to_string(char uuid[12], char hexstr[24]);
time_t uuid_get_time(char uuid[12]);

#ifdef __cplusplus
}
#endif
#endif
