#ifndef SGN_WIN32_H_
#define SGN_WIN32_H_


#ifdef __cplusplus
extern "C" {
#endif

int sgn_get_device_id(char device_id[64]);
int sgn_get_app_path(char path[1024]);

#ifdef __cplusplus
}
#endif
#endif
