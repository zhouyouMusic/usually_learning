#if !(defined DISABLE_PROVISION) || defined __WIN32__

#ifndef SGN_SECURECONF_H_
#define SGN_SECURECONF_H_


#ifdef __cplusplus
extern "C" {
#endif

int sgn_secureconf_decrypt(const char *cipher, char **plain,  int *size);
int sgn_secureconf_encrypt(const char *plain,  char **cipher, int *size);
int sgn_secureconf_decrypt2(const char *cipher, char **plain,  int *size, const char *salt);
int sgn_secureconf_encrypt2(const char *plain,  char **cipher, int *size, const char *salt);

#ifdef __cplusplus
}
#endif
#endif

#endif
