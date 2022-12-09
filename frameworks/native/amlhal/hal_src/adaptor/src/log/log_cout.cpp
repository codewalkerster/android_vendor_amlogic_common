#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include "log_cout.h"


int _log_print(int prio, const char *tag, const char *pq_tag, const char *fmt, ...)
{
    char buf[DEFAULT_LOG_BUFFER_LEN];
    sprintf(buf, "[%s]:", pq_tag);
    int pq_tag_len = strlen(buf);

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf + pq_tag_len, DEFAULT_LOG_BUFFER_LEN - pq_tag_len, fmt, ap);
    va_end(ap);

    return __android_log_write(prio, tag, buf);
}

void Logger(unsigned int level,  unsigned int module,const char* fmt, ...)
{
    va_list ap;
    char buf[DEFAULT_LOG_BUFFER_LEN];
    va_start(ap, fmt);
    vsnprintf(buf, DEFAULT_LOG_BUFFER_LEN , fmt, ap);
    va_end(ap);

    #if 1 // ANDROID
    __android_log_write(getAndroidLogLevel(level), getModuleString(module), buf);
    #else
    printf("[%s] [%s] %s", getLogLevelString(level), getModuleString(module), buf);
    #endif

    return;
}

static char* getModuleString(unsigned int module)
{
    if (module == LOG_VPQ)
        return (char*)"VPQ";
    if (module == LOG_DI)
        return (char*)"DI";
    if (module == LOG_MEMC)
        return (char*)"MEMC";
    if (module == LOG_LCD)
        return (char*)"PANEL";
    if (module == LOG_LD)
        return (char*)"LOCALDIMMING";
    if (module == LOG_DISPLAY)
        return (char*)"DISPLAY";
    if (module == LOG_DV)
        return (char*)"DV";
    if (module == LOG_VDIN)
        return (char*)"VDIN";

    return (char*)"UNKNOWN MODULE";
}

static char* getLogLevelString(unsigned int level)
{
    if (level == LOGGER_DEBUG)
        return (char*)"D";
    if (level == LOGGER_VERBOSE)
        return (char*)"V";
    if (level == LOGGER_INFO)
        return (char*)"I";
    if (level == LOGGER_ERROR)
        return (char*)"E";

    return (char*)"NULL";
}

int getAndroidLogLevel(unsigned int level)
{
    if (level == LOGGER_DEBUG)
        return ANDROID_LOG_DEBUG;
    if (level == LOGGER_VERBOSE)
        return ANDROID_LOG_VERBOSE;
    if (level == LOGGER_INFO)
        return ANDROID_LOG_INFO;
    if (level == LOGGER_ERROR)
        return ANDROID_LOG_ERROR;

    return ANDROID_LOG_DEBUG;
}

