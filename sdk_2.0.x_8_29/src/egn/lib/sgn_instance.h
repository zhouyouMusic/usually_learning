#ifndef DISABLE_PROVISION

#ifndef SGN_INSTANCE_TOKEN_H_
#define SGN_INSTANCE_TOKEN_H_


#ifdef __cplusplus
extern "C" {
#endif

struct sgn_instance;

struct sgn_instance * sgn_instance_apply_for(char *app_key, int max);
int sgn_instance_give_back(struct sgn_instance *token);


#ifdef __cplusplus
}
#endif
#endif

#endif
