#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "pq/aml_hal_di.h"
#include "pq/adap_di.h"

#ifdef __cplusplus
extern "C"
{
#endif

HAL_STATUS_T AML_HAL_DI_INIT(void)
{
    return API_OK;
}

HAL_STATUS_T AML_HAL_DI_Uninit(void)
{
    return API_OK;
}

HAL_STATUS_T AML_HAL_DI_Open(void)
{
    return API_OK;
}

HAL_STATUS_T AML_HAL_DI_Close(void)
{
    return API_OK;
}

#ifdef __cplusplus
}
#endif
