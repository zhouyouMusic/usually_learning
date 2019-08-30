/********************************************************************
 * Copyright 2017- Author: Frank Phillips All Rights Reserved.
 * NOTICE:  All information contained herein is, and remains
 * the property of the author. The intellectual and technical concepts 
 * contained herein are proprietary to the author. Dissemination of 
 * this information or reproduction of this material is strictly 
 * forbidden unless prior written permission is obtained from the author.
 */

// Copyright 2017 Frank Phillips 
// can not be copied and/or distributed without the express
// permission of Frank Phillips
// 
/** @file common-utils.h
 *  @brief Function prototypes for some common utils
 *
 *  @author Frank Phillips
 */

#ifndef __YY_COMMON_UTILS_H__
#define __YY_COMMON_UTILS_H__

#define yyeval_version "2.2.0"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Initialize global resources
 *  @param res_cfg_file configuration file location
 *
 */
__attribute ((visibility("default"))) void InitResource(char const *res_dir);

struct res_data
{
    char filename[24];  //文件名
    char comp;    //压缩率
    int datalen;   //数据长度
    struct res_data *next;
    char data[];   //数据
};

/**
 * @param rec 1 means need choice recognition
 */
__attribute ((visibility("default"))) void InitResource2(struct res_data *);

__attribute ((visibility("default"))) void DestroyResource();

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __YY_COMMON_UTILS_H__ */

