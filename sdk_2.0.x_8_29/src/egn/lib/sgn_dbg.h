#ifndef SGN_MG_DBG_H_
#define SGN_MG_DBG_H_

#include <stdio.h>

#ifndef SGN_ENABLE_DEBUG
#define SGN_ENABLE_DEBUG 0
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * Log level; `LOG_INFO` is the default. Use `sgn_log_set_level()` to change it.
 */
enum sgn_log_level {
  LOG_NONE = -1,
  LOG_ERROR = 0,
  LOG_WARN = 1,
  LOG_INFO = 2,
  LOG_DEBUG = 3,
  _LOG_MIN = -2,
  _LOG_MAX = 5,
};

/*
 * Set max log level to print; messages with the level above the given one will
 * not be printed.
 */
void sgn_log_set_level(enum sgn_log_level level);


/*
 * Helper function which prints message prefix with the given `level`, function
 * name `func` and `filename`. If message should be printed (accordingly to the
 * current log level and filter), prints the prefix and returns 1, otherwise
 * returns 0.
 *
 * Clients should typically just use `LOG()` macro.
 */
void sgn_log_print_prefix(enum sgn_log_level level, const char *filename, int line, const char *func, const char *fmt, ...);

extern enum sgn_log_level sgn_log_threshold;

/*
 * Set file to write logs into. If `NULOG`, logs go to `stderr`.
 */
void sgn_log_set_file(const char *output_path) ;

FILE *sgn_log_get_file();

//void sgn_log_printf(const char *fmt, ...);

void sgn_log_set_level(enum sgn_log_level level);

#ifndef SGN_NDEBUG

#ifdef __ANDROID__
    #define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)
    #define LOG(l, ...)                                                         \
      do {                                                                      \
        sgn_log_print_prefix(l, __FILENAME__, __LINE__, __func__, __VA_ARGS__);  \
      } while (0)
#else
    #define LOG(l, ...)                                                         \
      do {                                                                      \
        sgn_log_print_prefix(l, __FILE__, __LINE__, __func__, __VA_ARGS__);  \
      } while (0)
#endif
#define DBG(...) LOG(LOG_DEBUG, __VA_ARGS__)

#else /* NDEBUG */

#define LOG(l, x)
#define DBG(x)

#endif



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SGN_MG_DBG_H_ */
