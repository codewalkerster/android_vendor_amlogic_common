#ifndef _LOG_COUT_H_
#define _LOG_COUT_H_

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <log/log.h>

//1024 + 128
#define DEFAULT_LOG_BUFFER_LEN 1152

#define LOG_LEVEL_ERROR      0
#define LOG_LEVEL_WARNING    1
#define LOG_LEVEL_DEBUG      2
#define LOG_LEVEL_INFO       3

#define LOG_LEVEL_DEFAULT    2

typedef enum _LOGGER_LEVEL{
    LOGGER_ERROR    = 0,
    LOGGER_INFO     = 1,
    LOGGER_DEBUG    = 2,
    LOGGER_VERBOSE  = 3,
} LOGGER_LEVEL;

#define BIT(N)      (1<<N)

#define LOG_VPQ                   (BIT(0))
#define LOG_DI                    (BIT(1))
#define LOG_MEMC                  (BIT(2))
#define LOG_LCD                   (BIT(3))
#define LOG_LD                    (BIT(4))
#define LOG_DISPLAY               (BIT(5))
#define LOG_DV                    (BIT(6))
#define LOG_VDIN                  (BIT(7))
#define LOG_RESERVED0             (BIT(8))
#define LOG_RESERVED1             (BIT(9))
#define LOG_RESERVED2             (BIT(10))
#define LOG_RESERVED3             (BIT(11))
#define LOG_RESERVED4             (BIT(12))
#define LOG_RESERVED5             (BIT(13))
#define LOG_RESERVED6             (BIT(14))
#define LOG_RESERVED7             (BIT(15))
#define LOG_RESERVED8             (BIT(16))
#define LOG_RESERVED9             (BIT(17))
#define LOG_RESERVED10            (BIT(18))
#define LOG_RESERVED11            (BIT(19))
#define LOG_RESERVED12            (BIT(20))
#define LOG_RESERVED13            (BIT(21))
#define LOG_RESERVED14            (BIT(22))
#define LOG_RESERVED15            (BIT(23))
#define LOG_RESERVED16            (BIT(24))
#define LOG_RESERVED17            (BIT(25))
#define LOG_RESERVED18            (BIT(26))
#define LOG_RESERVED19            (BIT(27))
#define LOG_RESERVED20            (BIT(28))
#define LOG_RESERVED21            (BIT(29))
#define LOG_RESERVED22            (BIT(30))
#define LOG_RESERVED23            (BIT(31))

extern int _log_print(int prio, const char *tag, const char *pq_tag, const char *fmt, ...);
extern void Logger(unsigned int level,  unsigned int module,const char* fmt, ...);

static char* getModuleString(unsigned int module);

int getAndroidLogLevel(unsigned int level);
static char* getLogLevelString(unsigned int level);


#define AML_LOG_DEBUG(module, fmt, arg...)    Logger(LOGGER_DEBUG, module, fmt, ##arg)
#define AML_LOG_VERBOSE(module, fmt, arg...)  Logger(LOGGER_VERBOSE, module, fmt, ##arg)
#define AML_LOG_ERROR(module, fmt, arg...)    Logger(LOGGER_ERROR, module, fmt, ##arg)
#define AML_LOG_INFO(module, fmt, arg...)     Logger(LOGGER_INFO, module, fmt, ##arg)


#undef LOGE
#define LOGE(...) \
    _log_print(ANDROID_LOG_ERROR, "AML_HAL", "E", __VA_ARGS__)

#undef LOGV
#define LOGV(...) \
    _log_print(ANDROID_LOG_VERBOSE, "AML_HAL", "W", __VA_ARGS__)

#undef LOGD
#define LOGD(...) \
    _log_print(ANDROID_LOG_DEBUG, "AML_HAL", "D", __VA_ARGS__)

#undef LOGI
#define LOGI(...) \
    _log_print(ANDROID_LOG_INFO, "AML_HAL", "I", __VA_ARGS__)



#endif //_LOG_COUT_H_