/*******************************************************************************
* Copyright (C),  
* 
* FileName    : sgn_file.h
* Author      :  
*  
* Description : 操作系统相关的文件操作接口封装 
*------------------------------------------------------------------------------*
* Record      : 
*******************************************************************************/
#ifndef SGN_FILE_H_
#define SGN_FILE_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __WIN32__
#include <windows.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int sgn_is_file_exist(const char *filepath);

#ifdef __WIN32__
void sgn_utf8_to_ansi(const char  *utf8, char *buf);
#endif

#ifdef __cplusplus
}
#endif
#endif
