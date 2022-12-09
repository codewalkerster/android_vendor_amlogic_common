#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "vdin/aml_hal_vdin.h"
#include "vdin/adap_vdin.h"


#ifdef __cplusplus
extern "C"
{
#endif


HAL_STATUS_T AML_HAL_VDIN_INIT(void)
{
    if (ADAP_VDIN_INIT() == ADAP_OK)
        return API_OK;
    else
        return API_NOT_OK;
}

HAL_STATUS_T AML_HAL_VDIN_Uninit(void)
{
    if (ADAP_VDIN_Uninit() == ADAP_OK)
        return API_OK;
    else
        return API_NOT_OK;
}

HAL_STATUS_T AML_HAL_VDIN_Open(void)
{
    if (ADAP_VDIN_Open() == ADAP_OK)
        return API_OK;
    else
        return API_NOT_OK;
}

HAL_STATUS_T AML_HAL_VDIN_Close(void)
{
    if (ADAP_VDIN_Close() == ADAP_OK)
        return API_OK;
    else
        return API_NOT_OK;
}

#ifdef __cplusplus
}
#endif
