#ifndef _ADAP_COMMON_H_
#define _ADAP_COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "log/log_cout.h"

#ifdef __cplusplus
extern "C" {
#endif


#ifndef UINT8
typedef unsigned char           _UINT8;
#define UINT8 _UINT8
#endif

#ifndef SINT8
typedef signed char             _SINT8;
#define SINT8 _SINT8
#endif

#ifndef CHAR
typedef char                    _CHAR;
#define CHAR _CHAR
#endif

#ifndef UINT16
typedef unsigned short          _UINT16;
#define UINT16 _UINT16
#endif

#ifndef SINT16
typedef signed short            _SINT16;
#define SINT16 _SINT16
#endif

#ifndef UINT32
typedef unsigned int            _UINT32;
#define UINT32 _UINT32
#endif

#ifndef SINT32
typedef signed int              _SINT32;
#define SINT32 _SINT32
#endif

#ifndef BOOLEAN
typedef unsigned char           _BOOLEAN;
#define BOOLEAN _BOOLEAN
#endif

#ifndef ULONG
typedef unsigned long           _ULONG;
#define ULONG _ULONG
#endif

#ifndef SLONG
typedef signed long             _SLONG;
#define SLONG _SLONG
#endif

#ifndef UINT64
typedef unsigned long long      _UINT64;
#define UINT64 _UINT64
#endif

#ifndef SINT64
typedef signed long long        _SINT64;
#define SINT64 _SINT64
#endif



#ifndef TRUE
#define TRUE                    (1)
#endif

#ifndef FALSE
#define FALSE                   (0)
#endif

#ifndef ON_STATE
#define ON_STATE                (1)
#endif

#ifndef OFF_STATE
#define OFF_STATE               (0)
#endif

#ifndef ON
#define ON                      (1)
#endif

#ifndef OFF
#define OFF                     (0)
#endif

#ifndef NULL
#define NULL                    ((void *)0)
#endif


#ifndef ADAP_STATE_T

typedef enum
{
    ADAP_OK                      =  0,
    ADAP_NOT_OK                  = -1,
    ADAP_INVALID_PARAMS          = -2,
    ADAP_NOT_ENOUGH_RESOURCE     = -3,
    ADAP_NOT_SUPPORTED           = -4,
    ADAP_NOT_PERMITTED           = -5,
    ADAP_TIMEOUT                 = -6,
}_ADAP_STATE_T;


#define ADAP_STATE_T             _ADAP_STATE_T

#endif

typedef ADAP_STATE_T    ADAP_STATUS_T;

#define CHECK_EXPRESSION_RET(exp)    \
    if (exp)    \
    {    \
        LOGE("%s %d Param is Illegal!\n", __FUNCTION__, __LINE__);    \
        return API_INVALID_PARAMS;    \
    }

#ifdef __cplusplus
}
#endif
#endif
