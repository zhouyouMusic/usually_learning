/*
 * Copyright (c) 2014-2018 Cesanta Software Limited
 * All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the ""License"");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ""AS IS"" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "sgn_dbg.h"
#include "sgn_file.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

enum sgn_log_level sgn_log_threshold  = LOG_ERROR;

char level_str[4][10] = {"LOG_ERROR", "LOG_WARN", "LOG_INFO", "LOG_DEBUG"};

FILE *sgn_log_file  = NULL;

#ifdef __ANDROID__
#   include <android/log.h>
#endif

void sgn_log_print_prefix(enum sgn_log_level level, const char *filename, int line, const char *func, const char *fmt, ...)
{
  va_list ap;
  if (level > sgn_log_threshold) return;
  if (sgn_log_file == NULL) sgn_log_file = stderr;
  struct timeval _tv;
  char buf[1024] = {0};
  time_t _tm_t = 0;
  struct tm _tm = { 0 };
  gettimeofday(&_tv, NULL);
  _tm_t = _tv.tv_sec;
  if (gmtime(&_tm_t)) _tm = *gmtime(&_tm_t);
  sprintf(buf, "%02d-%02d-%02d %02d:%02d:%02d.%03d ", _tm.tm_year + 1900, _tm.tm_mon + 1, _tm.tm_mday, _tm.tm_hour + 8, _tm.tm_min, _tm.tm_sec, (int)_tv.tv_usec/1000);
#ifdef __ANDROID__
  if(sgn_log_file > stderr){
#endif
  fprintf(sgn_log_file, "%10s |%s %s#%d %s() ", level_str[level], buf, filename, line, func);
#ifdef __ANDROID__
  }else{
        __android_log_print(ANDROID_LOG_DEBUG, "skegn","%10s |%s %s#%d %s()", level_str[level], buf, filename, line, func);
    }
#endif
//  va_list ap;
  va_start(ap, fmt);
#ifdef __ANDROID__
  if(sgn_log_file > stderr){
#endif
  vfprintf(sgn_log_file, fmt, ap);
#ifdef __ANDROID__
  }else{
      __android_log_vprint(ANDROID_LOG_DEBUG, "skegn", fmt, ap);
  }
#endif
  va_end(ap);
// #ifndef __ANDROID__
  fputc('\n', sgn_log_file);
  fflush(sgn_log_file);
// #endif
}

void sgn_log_printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
#ifdef __ANDROID__
  if(sgn_log_file > stderr){
#endif
  vfprintf(sgn_log_file, fmt, ap);
#ifdef __ANDROID__
  }else{
      __android_log_vprint(ANDROID_LOG_DEBUG, "skegn", fmt, ap);
  }
#endif
  va_end(ap);
#ifndef __ANDROID__
  fputc('\n', sgn_log_file);
  fflush(sgn_log_file);
#endif
}

void sgn_log_set_file(const char *output_path) {
    if (output_path && strcmp(output_path, "")) {
#ifdef __WIN32__
        sgn_utf8_to_ansi(output_path, output_path);
#endif
        sgn_log_file = fopen(output_path, "w");
    }
}

FILE *sgn_log_get_file()
{
  return sgn_log_file>stderr ? sgn_log_file : NULL;
}

void sgn_log_set_level(enum sgn_log_level level) {
  sgn_log_threshold = level;
}
