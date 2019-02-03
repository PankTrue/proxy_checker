#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <execinfo.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>



#define BT_BUF_SIZE 128

enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL, LOG_FATAL_WITH_BACKTRACE };

#define log_trace(...) log_log(LOG_TRACE, "", 0, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, "", 0, __VA_ARGS__)
#define log_info(...)  log_log(LOG_INFO,  "", 0, __VA_ARGS__)
#define log_warn(...)  log_log(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__); exit(1)
#define log_fatal_with_backtrace(...) log_log(LOG_FATAL_WITH_BACKTRACE, __FILE__, __LINE__, __VA_ARGS__); print_backtrace(); exit(1)

void log_set_udata(void *udata);
void log_set_fp(FILE *fp);
void log_set_level(int level);
void log_set_quiet(int enable);

void log_log(int level, const char *file, int line, const char *fmt, ...);


void print_backtrace();

#endif
