#include "log.h"

static pthread_mutex_t lock;

static struct {
  pthread_mutex_t lock;
  FILE *fp;
  int level;
  int quiet;
} L;

static const char *level_names[] = {
  "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

static const char *level_colors[] = {
  "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};

void log_set_fp(FILE *fp)
{
  L.fp = fp;
}

void log_set_level(int level)
{
  L.level = level;
}

void log_set_quiet(int enable)
{
  L.quiet = enable;
}



void log_log(int level, const char *file, int line, const char *fmt, ...)
{
    char buf[64];
    if (level < L.level) { return; }

    pthread_mutex_lock(&lock);

    time_t t = time(NULL);
    struct tm *lt = localtime(&t);

    if (!L.quiet)
    {
        va_list args;
        buf[strftime(buf, sizeof(buf), "%H:%M:%S", lt)] = '\0';

        fprintf(stderr, "%s %s%-5s\x1b[0m\x1b[90m%s:%d:\x1b[0m ",
            buf, level_colors[level], level_names[level], file, line);

        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);
        fprintf(stderr, "\n");
        fflush(stderr);
    }

    if (L.fp)
    {
        va_list args;
        buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt)] = '\0';
        fprintf(L.fp, "%s %-5s %s:%d: ", buf, level_names[level], file, line);
        va_start(args, fmt);
        vfprintf(L.fp, fmt, args);
        va_end(args);
        fprintf(L.fp, "\n");
        fflush(L.fp);
    }


    pthread_mutex_unlock(&lock);
}

void print_backtrace()
{
    pthread_mutex_lock(&lock);
    int j, nptrs;
    void *buffer[BT_BUF_SIZE];
    char **strings;

    nptrs = backtrace(buffer, BT_BUF_SIZE);
    printf("backtrace() returned %d addresses\n", nptrs);

    strings = backtrace_symbols(buffer, nptrs);
    if (strings == NULL)
    {
        perror("backtrace_symbols");
        exit(EXIT_FAILURE);
    }

    for (j = 0; j < nptrs; j++)
        printf("%s\n", strings[j]);

    free(strings);

    pthread_mutex_unlock(&lock);
}
