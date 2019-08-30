#ifndef DISABLE_PROVISION

#ifdef USE_NATIVE
#ifndef SGN_AUTH_H_
#define SGN_AUTH_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <time.h>
#define SGN_AUTH_NULL               0
#define SGN_AUTH_EXPIRE_LESS_18H    1
#define SGN_AUTH_EXPIRE_LESS_30D    2
#define SGN_AUTH_NEED_ACTIVATE      3
#define SGN_AUTH_EXPIRE_OK          10
#define SGN_AUTH_USE_CATFISH        11


struct sgn_provision;

struct sgn_provision *sgn_provision_new(const char *path);
int sgn_provision_delete(struct sgn_provision *provision);
int sgn_auth_verify(struct sgn_provision *provision, const char *app_key, const char *secret_key, const char *core_type, char **error);
int sgn_get_serialNumber(const char *appkey, const char *deviceId, const char *userId, const char *secret_key, char serialNumber[64]);
int sgn_check_provision(struct sgn_provision *provision);
const char *sgn_provision_get_auth_addr(struct sgn_provision *provision);
void sgn_provision_set_auth_addr(struct sgn_provision *provision, const char *addr);
void sgn_provision_set_catfish_auth(struct sgn_provision *provision, time_t value);
time_t sgn_provision_get_catfish_auth(struct sgn_provision *provision);
void sgn_provision_set_catfish_connected(struct sgn_provision *provision, int value);
int sgn_provision_get_catfish_connected(struct sgn_provision *provision);
void sgn_activate_provision(struct sgn_provision *provision, const char *path);
void sgn_auth_set_expire(struct sgn_provision *provision, const char *path);

#ifdef __cplusplus
}
#endif
#endif

#endif

#endif
