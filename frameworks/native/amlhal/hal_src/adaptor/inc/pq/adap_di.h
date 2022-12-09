#ifndef _ADAP_DI_H_
#define _ADAP_DI_H_

#include <adap_common.h>

typedef struct am_pq_param_s {
    unsigned int table_name;
    unsigned int table_len;
    union {
        void *table_ptr;
        long long table_ptr_len;
    };

    union {
        void *reserved0;
        long long reserved0_len;
    };
} am_pq_param_t;

ADAP_STATUS_T ADAP_DI_INIT(void);
ADAP_STATUS_T ADAP_DI_Uninit(void);
ADAP_STATUS_T ADAP_DI_Open(void);
ADAP_STATUS_T ADAP_DI_Close(void);

#endif
